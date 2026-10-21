#include "EmberDSP.h"

namespace ember
{
namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    // Hyperbolic tangent, Pade (7/6) approximation. Accurate to better than
    // 1e-6 for |x| < 2 and monotone everywhere we need it, but far cheaper than
    // std::tanh on the hot path.
    inline float tanhApprox (float x) noexcept
    {
        const float ax = x < 0.0f ? -x : x;
        if (ax > 6.0f)
            return x < 0.0f ? -1.0f : 1.0f;
        const float x2 = x * x;
        const float num = x * (135135.0f + x2 * (17325.0f + x2 * (378.0f + x2)));
        const float den = 135135.0f + x2 * (62370.0f + x2 * (3150.0f + x2 * 28.0f));
        return num / den;
    }

    inline float softSat (float x, float drive) noexcept
    {
        return tanhApprox (x * drive) / drive;
    }

    // Smooth limiter with unity gain at low level and a soft ceiling at c.
    // Unlike tanh this leaves the operating point alone, so raising the drive
    // adds harmonics without changing the level - exactly what is wanted for a
    // valve stage whose bias is being moved by the sidechain.
    inline float softLimit (float x, float ceiling) noexcept
    {
        const float c = ceiling < 0.1f ? 0.1f : ceiling;
        const float t = (x / c) * (x / c);
        const float den = std::sqrt (std::sqrt (1.0f + t * t));
        return x / den;
    }

    inline float onePoleCoef (double fs, float hz) noexcept
    {
        const float t = 1.0f - std::exp (-2.0f * kPi * hz / static_cast<float> (fs));
        return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    }

    inline float timeCoef (double fs, float ms) noexcept
    {
        const float samples = std::max (1.0f, static_cast<float> (fs) * std::max (ms, 0.045f) * 0.001f);
        return 1.0f - std::exp (-1.0f / samples);
    }

    inline float besselI0 (float x) noexcept
    {
        const float y = x * x * 0.25f;
        float sum = 1.0f, term = 1.0f;
        for (int k = 1; k < 24; ++k)
        {
            term *= y / static_cast<float> (k * k);
            sum += term;
        }
        return sum;
    }

    inline float sinc (float x) noexcept
    {
        if (std::abs (x) < 1.0e-7f)
            return 1.0f;
        const float px = kPi * x;
        return std::sin (px) / px;
    }

    inline float clampf (float v, float lo, float hi) noexcept
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }
}

//==============================================================================
// Half band interpolation / decimation filters. 63 taps, 8.5 dB short of
// nothing in the stop band, but 31 non zero taps per phase so a 4x oversampled
// channel costs roughly 60 multiply/adds per input sample per filter.
void Processor::HalfBand::design()
{
    constexpr float beta = 9.0f;
    float sum = 0.0f;

    for (int j = 0; j < kHalfBandTaps; ++j)
    {
        const float offset = static_cast<float> (j - kHalfBandCentre);
        const float x = static_cast<float> (j) / static_cast<float> (kHalfBandTaps - 1) * 2.0f - 1.0f;
        const float window = besselI0 (beta * std::sqrt (std::max (0.0f, 1.0f - x * x))) / besselI0 (beta);

        // a half-band prototype has zero taps at every EVEN offset from the
        // centre; the odd offsets carry the interpolation phase
        h[j] = (j == kHalfBandCentre) ? 0.5f : 0.5f * sinc (0.5f * offset) * window;
        if (((j - kHalfBandCentre) & 1) == 0 && j != kHalfBandCentre)
            h[j] = 0.0f;

        sum += h[j];
    }

    // Unity DC gain, keeping the centre tap exact so the polyphase shortcuts
    // used below stay valid.
    const float rest = sum - h[kHalfBandCentre];
    const float scale = rest > 1.0e-9f ? (1.0f - h[kHalfBandCentre]) / rest : 1.0f;

    for (int j = 0; j < kHalfBandTaps; ++j)
        if (j != kHalfBandCentre)
            h[j] *= scale;

    for (int k = 0; k < kHalfBandPhase; ++k)
        even[k] = h[2 * k];

    reset();
}

void Processor::HalfBand::reset() noexcept
{
    for (auto& v : hist)
        v = 0.0f;
    write = 0;
}

void Processor::HalfBand::processUp (const float* in, float* out, int n) noexcept
{
    // Output pair for input sample i:
    //   even phase: 2 * sum p[2k] x[i-k]
    //   odd  phase: 2 * p[31] * x[i-15]
    constexpr int mask = kHalfBandRing - 1;
    constexpr int centre = kHalfBandCentre;

    for (int i = 0; i < n; ++i)
    {
        hist[static_cast<size_t> (write & mask)] = in[i];
        const int idx = write++;

        float acc = 0.0f;
        for (int k = 0; k < kHalfBandPhase; ++k)
            acc += even[k] * hist[static_cast<size_t> ((idx - k) & mask)];

        out[2 * i] = 2.0f * acc;
        out[2 * i + 1] = 2.0f * h[centre] * hist[static_cast<size_t> ((idx - (centre - 1) / 2) & mask)];
    }
}

void Processor::HalfBand::processDown (const float* in, float* out, int n) noexcept
{
    // y[i] = sum_k p[2k] x[2i-2k] + p[31] x[2i-31]
    constexpr int mask = kHalfBandRing - 1;

    for (int i = 0; i < n; ++i)
    {
        hist[static_cast<size_t> (write & mask)] = in[2 * i];
        const int base = write++;
        hist[static_cast<size_t> (write & mask)] = in[2 * i + 1];
        ++write;

        float acc = h[kHalfBandCentre] * hist[static_cast<size_t> ((base - kHalfBandCentre) & mask)];
        for (int k = 0; k < kHalfBandPhase; ++k)
            acc += even[k] * hist[static_cast<size_t> ((base - 2 * k) & mask)];

        out[i] = acc;
    }
}

//==============================================================================
void Processor::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    maxBlockSize_ = std::max (16, maxBlockSize);
    numChannels_ = std::min (kMaxChannels, std::max (1, numChannels));
    stages_ = sampleRate_ >= 120000.0 ? 1 : 2;

    for (int ch = 0; ch < kMaxChannels; ++ch)
    {
        for (int stage = 0; stage < 2; ++stage)
        {
            up_[ch][stage].design();
            down_[ch][stage].design();
        }
    }

    for (auto& ch : osCh_)
        ch.assign (static_cast<size_t> (maxBlockSize_ * 4 + 8), 0.0f);
    for (auto& ch : scratch_)
        ch.assign (static_cast<size_t> (maxBlockSize_ * 4 + 8), 0.0f);

    // Interpolators and decimators each contribute their group delay; the
    // measured peak of the chain's impulse response is 46 samples at 4x and 31
    // at 2x, which is what has to be reported to the host.
    const float baseDelay = stages_ == 2 ? (31.0f / 2.0f + 31.0f / 4.0f) * 2.0f
                                         : (31.0f / 2.0f) * 2.0f;
    latency_ = std::max (1, static_cast<int> (std::floor (baseDelay)));

    // the sidechain sees the signal after interpolation only
    const float scDelay = stages_ == 2 ? (31.0f / 2.0f + 31.0f / 4.0f) : (31.0f / 2.0f);
    sidechainDelay_ = static_cast<int> (std::lround (scDelay));

    for (auto& ch : dryBuf_)
        ch.assign (static_cast<size_t> (std::max (maxBlockSize_ + latency_ + 8, 64)), 0.0f);

    dryWrite_ = 0;
    reset();
}

void Processor::reset()
{
    for (auto& pair : up_)
        for (auto& hb : pair)
            hb.reset();
    for (auto& pair : down_)
        for (auto& hb : pair)
            hb.reset();
    for (auto& s : state_)
        s = ChannelState {};

    for (auto& ch : osCh_)
        std::fill (ch.begin(), ch.end(), 0.0f);

    for (auto& ch : dryBuf_)
        std::fill (ch.begin(), ch.end(), 0.0f);

    dryWrite_ = 0;
    snapParams_ = true;
    inGain_ = outGain_ = charPre_ = charPost_ = mix_ = 1.0f;
    meterGr_ = 0.0f;
    meterOut_ = -100.0f;
    vuState_ = 0.0f;
}

void Processor::updateRates (const Parameters& p) noexcept
{
    const double fs = sampleRate_ * (stages_ == 2 ? 4.0 : 2.0);
    const float attackMs = p.attackMs > 0.0f ? clampf (p.attackMs, 0.05f, 4.0f) : 0.0f;
    const float releaseMs = clampf (p.releaseMs, 5.0f, 2000.0f);

    attackCoef_ = timeCoef (fs, std::max (attackMs, 0.02f));
    relFastCoef_ = timeCoef (fs, std::max (releaseMs * 0.35f, 2.0f));
    relSlowCoef_ = timeCoef (fs, std::max (releaseMs * 1.25f, 4.0f));
    rectCoef_ = timeCoef (fs, clampf (releaseMs * 0.25f, 6.0f, 130.0f));
    rectFastCoef_ = timeCoef (fs, 0.004f);
    scHpCoef_ = onePoleCoef (fs, 160.0f);

    // 300 ohm position loads the input transformer harder
    ironDrive_ = p.lowZInput ? 1.22f : 1.0f;
    ironComp_ = 1.0f / ironDrive_;

    limiting_ = attackMs > 0.0f && ! p.bypass;
    linked_ = p.stereoLink;
    detectorHighPass_ = p.detectorHighPass;
}

void Processor::process (float* const* channels, int numChannels, int numSamples, const Parameters& p)
{
    if (channels == nullptr || numSamples <= 0)
        return;

    // Hosts are supposed to respect the block size we were prepared with, but
    // never trust them: split anything larger so the internal buffers stay in
    // bounds.
    int offset = 0;
    while (offset < numSamples)
    {
        const int chunk = std::min (maxBlockSize_, numSamples - offset);
        float* ptrs[kMaxChannels] { nullptr, nullptr };
        for (int ch = 0; ch < std::min (numChannels, kMaxChannels); ++ch)
            ptrs[ch] = channels[ch] + offset;

        processChunk (ptrs, numChannels, chunk, p);
        offset += chunk;
    }
}

void Processor::processChunk (float* const* channels, int numChannels, int numSamples, const Parameters& p)
{
    const int n = std::min (numSamples, maxBlockSize_);
    const int numCh = std::min (std::max (1, numChannels), numChannels_);
    const double fsBase = sampleRate_;

    updateRates (p);

    thresholdDb_ = model::kThresholdDbfs + clampf (p.headroomDb, -24.0f, 24.0f)
                                        - clampf (p.grTrimDb, -12.0f, 12.0f);
    ratioKnee_ = model::kRatioAtKnee;
    ratioGrowth_ = model::kRatioGrowth;
    maxGr_ = model::kMaxGainReductionDb;

    const float targetIn = model::dbToGain (clampf (p.inputDb + p.inputTrimDb, -24.0f, 56.0f)
                                            - model::kAttenuatorReferenceDb);
    const float targetOut = model::dbToGain (clampf (p.outputDb + p.outputTrimDb, -24.0f, 56.0f)
                                             - model::kAttenuatorReferenceDb);
    const float targetMix = clampf (p.mix, 0.0f, 1.0f);

    // Headroom moves the internal operating level: more headroom lifts the
    // limiting threshold and shrinks the drive into the valve stages while the
    // static gain is left untouched.
    const float hr = clampf (p.headroomDb, -24.0f, 24.0f);
    const float targetCharPre = model::dbToGain (-0.5f * hr);
    const float targetCharPost = 1.0f / targetCharPre;

    const float inCoef = timeCoef (fsBase, 15.0f);
    const float outCoef = inCoef;

    if (p.bypass)
    {
        // True bypass: the dry signal is still delayed so that switching is
        // click free and the latency stays constant.
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* outCh = channels[ch];
            auto& dry = dryBuf_[static_cast<size_t> (ch)];
            const int len = static_cast<int> (dry.size());
            for (int i = 0; i < n; ++i)
            {
                const float x = outCh[i];
                dry[static_cast<size_t> (dryWrite_)] = x;
                outCh[i] = dry[static_cast<size_t> ((dryWrite_ - latency_ + len) % len)];
                dryWrite_ = (dryWrite_ + 1) % len;
            }
        }
        for (auto& s : state_)
        {
            s.gain = 1.0f;
            s.cv1 = s.cv2 = s.grDb = s.env = 0.0f;
        }
        meterGr_ = 0.0f;
        return;
    }

    // ---- base rate: input gain, dry delay tap ------------------------------
    if (snapParams_)
    {
        // first block after a reset: start at the requested settings instead of
        // ramping from unity, so that offline renders and tests are exact
        inGain_ = targetIn;
        outGain_ = targetOut;
        mix_ = targetMix;
        charPre_ = targetCharPre;
        charPost_ = targetCharPost;
        snapParams_ = false;
    }

    for (int i = 0; i < n; ++i)
    {
        inGain_ += (targetIn - inGain_) * inCoef;
        outGain_ += (targetOut - outGain_) * outCoef;
        mix_ += (targetMix - mix_) * inCoef;
        charPre_ += (targetCharPre - charPre_) * inCoef;
        charPost_ += (targetCharPost - charPost_) * inCoef;

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto& dry = dryBuf_[static_cast<size_t> (ch)];
            const int len = static_cast<int> (dry.size());
            const float x = channels[ch][i];
            dry[static_cast<size_t> (dryWrite_)] = x;
            const float delayed = dry[static_cast<size_t> ((dryWrite_ - latency_ + len) % len)];
            channels[ch][i] = delayed;                 // temporarily hold dry
            osCh_[static_cast<size_t> (ch)][static_cast<size_t> (i)] = x * inGain_ * charPre_;
        }
        dryWrite_ = (dryWrite_ + 1) % static_cast<int> (dryBuf_[0].size());
    }

    // ---- upsample ----------------------------------------------------------
    for (int ch = 0; ch < numCh; ++ch)
    {
        float* buf = osCh_[static_cast<size_t> (ch)].data();
        float* temp = scratch_[static_cast<size_t> (ch)].data();
        if (stages_ == 2)
        {
            up_[ch][0].processUp (buf, temp, n);
            up_[ch][1].processUp (temp, buf, n * 2);
        }
        else
        {
            up_[ch][0].processUp (buf, temp, n);
            std::copy (temp, temp + n * 2, buf);
        }
    }

    const int n4 = n * (stages_ == 2 ? 4 : 2);
    const double fsOs = fsBase * (stages_ == 2 ? 4.0 : 2.0);
    const float xfHp = onePoleCoef (fsOs, 10.0f);
    const float ironLp = onePoleCoef (fsOs, 90.0f);
    const float ampHp = onePoleCoef (fsOs, 6.0f);
    const float outHp = onePoleCoef (fsOs, 12.0f);

    // ---- oversampled non linear chain --------------------------------------
    for (int i = 0; i < n4; ++i)
    {
        float tap[kMaxChannels] {};
        float drive_[kMaxChannels] {};

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto& s = state_[ch];
            const float stageInput = osCh_[static_cast<size_t> (ch)][static_cast<size_t> (i)];
            float x = stageInput;

            // input transformer: coupling, 2nd order asymmetry, LF core saturation
            float hp = x - s.xfHpZ;
            s.xfHpZ = flushDenormal (s.xfHpZ + xfHp * hp);
            s.lfZ = flushDenormal (s.lfZ + ironLp * (hp - s.lfZ));
            const float lf = s.lfZ;
            const float lfSat = softSat (lf * ironDrive_, 1.1f) * ironComp_;
            x = (hp - lf) + lfSat;

            // 6BC8 grid curvature: a little even order from the triode mismatch
            x = x + 0.010f * x * x;
            x = softLimit (x, 1.9f);

            // variable mu gain element
            // the 6AL5 sidechain is fed from the transformer secondary, so it
            // sees exactly the signal that arrives at the gain element. The
            // LOW / HI switch can high-pass that feed so bass does not pump the
            // whole mix.
            if (detectorHighPass_)
            {
                s.scHpZ = flushDenormal (s.scHpZ + scHpCoef_ * (x - s.scHpZ));
                drive_[ch] = x - s.scHpZ;
            }
            else
            {
                drive_[ch] = x;
            }

            float y = x * s.gain;

            // the valve curves harder as bias (and therefore GR) increases
            const float grAmount = s.grDb;
            y = softLimit (y, std::max (0.9f, 1.7f - 0.010f * grAmount));

            // 12AX7 / 12BH7 amplifier stage
            float a = y - s.dcZ;
            s.dcZ = flushDenormal (s.dcZ + ampHp * a);
            a = softLimit (a, 1.6f);
            float amp = a - s.ampZ;
            s.ampZ = flushDenormal (s.ampZ + outHp * amp);

            tap[ch] = amp;
        }

        // ---- 6AL5 sidechain ------------------------------------------------
        if (linked_)
        {
            float rect = std::abs (drive_[0]);
            for (int ch = 1; ch < numCh; ++ch)
                rect = std::max (rect, std::abs (drive_[ch]));

            auto& s = state_[0];
            if (! limiting_)
            {
                s.env = flushDenormal (s.env + rectFastCoef_ * (rect - s.env));
                s.cv1 = s.cv2 = 0.0f;
                s.grDb = 0.0f;
                s.gain = 1.0f;
            }
            else
            {
                s.env = flushDenormal (s.env + (rect > s.env ? rectFastCoef_ : rectCoef_) * (rect - s.env));
                const float levelDb = 20.0f * std::log10 (std::max (s.env, 1.0e-7f));

                // The 6AL5 charges a control voltage through the attack and
                // release network. That voltage tracks a LEVEL, not a gain, so
                // the ballistics live in the dB domain: first the attack
                // capacitor, then the long tail that makes the unit program
                // dependent. The 6BC8 grid then turns CV into gain.
                s.cv1 += (levelDb > s.cv1 ? attackCoef_ : relFastCoef_) * (levelDb - s.cv1);
                s.cv2 += (s.cv1 > s.cv2 ? attackCoef_ : relSlowCoef_) * (s.cv1 - s.cv2);

                const float u = s.cv2 - thresholdDb_;
                float grDb = 0.0f;
                if (u > 0.0f)
                    grDb = std::min (maxGr_, u * (1.0f - 1.0f / (ratioKnee_ + ratioGrowth_ * u)));

                s.grDb = grDb;
                s.gain = model::dbToGain (-grDb);
            }

            for (int ch = 0; ch < numCh; ++ch)
            {
                state_[ch].gain = s.gain;
                state_[ch].cv1 = s.cv1;
                state_[ch].cv2 = s.cv2;
            }
        }
        else
        {
            for (int ch = 0; ch < numCh; ++ch)
            {
                auto& s = state_[ch];
                const float rect = std::abs (drive_[ch]);
                if (! limiting_)
                {
                    s.cv1 = s.cv2 = 0.0f;
                    s.grDb = 0.0f;
                    s.gain = 1.0f;
                    continue;
                }

                s.env = flushDenormal (s.env + (rect > s.env ? rectFastCoef_ : rectCoef_) * (rect - s.env));
                const float levelDb = 20.0f * std::log10 (std::max (s.env, 1.0e-7f));

                // The 6AL5 charges a control voltage through the attack and
                // release network. That voltage tracks a LEVEL, not a gain, so
                // the ballistics live in the dB domain: first the attack
                // capacitor, then the long tail that makes the unit program
                // dependent. The 6BC8 grid then turns CV into gain.
                s.cv1 += (levelDb > s.cv1 ? attackCoef_ : relFastCoef_) * (levelDb - s.cv1);
                s.cv2 += (s.cv1 > s.cv2 ? attackCoef_ : relSlowCoef_) * (s.cv1 - s.cv2);

                const float u = s.cv2 - thresholdDb_;
                float grDb = 0.0f;
                if (u > 0.0f)
                    grDb = std::min (maxGr_, u * (1.0f - 1.0f / (ratioKnee_ + ratioGrowth_ * u)));

                s.grDb = grDb;
                s.gain = model::dbToGain (-grDb);
            }
        }

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto& s = state_[ch];
            float y = tap[ch];

            // output transformer: LF core saturation and coupling capacitor
            float hp = y - s.outZ;
            s.outZ = flushDenormal (s.outZ + outHp * hp);
            y = hp;
            y = y + 0.006f * y * y;
            y = softLimit (y, 1.4f);

            osCh_[static_cast<size_t> (ch)][static_cast<size_t> (i)] = y;
        }
    }

    // ---- downsample and blend ---------------------------------------------
    float peak = 0.0f;
    float grSum = 0.0f;

    for (int ch = 0; ch < numCh; ++ch)
    {
        float* buf = osCh_[static_cast<size_t> (ch)].data();
        float* temp = scratch_[static_cast<size_t> (ch)].data();
        if (stages_ == 2)
        {
            down_[ch][1].processDown (buf, temp, n4 / 2);
            down_[ch][0].processDown (temp, buf, n);
        }
        else
        {
            down_[ch][0].processDown (buf, temp, n);
            std::copy (temp, temp + n, buf);
        }

        float* outCh = channels[ch];
        const float outGain = outGain_ * charPost_;
        for (int i = 0; i < n; ++i)
        {
            const float wet = buf[i] * outGain;
            const float dry = outCh[i];
            const float y = dry + (wet - dry) * mix_;
            outCh[i] = y;
            peak = std::max (peak, std::abs (y));
        }

        grSum += state_[ch].grDb;
    }

    // ---- metering ----------------------------------------------------------
    const float gr = grSum / static_cast<float> (std::max (1, numCh));
    instantGr_ = gr;
    // block rate smoothing so the needle behaves the same at any buffer size
    const float blockSeconds = static_cast<float> (n) / static_cast<float> (fsBase);
    meterGr_ += (gr - meterGr_) * (1.0f - std::exp (-blockSeconds / 0.020f));

    const float peakDb = 20.0f * std::log10 (std::max (peak, 1.0e-7f));
    vuState_ += (peakDb - vuState_) * (1.0f - std::exp (-1.0f / (static_cast<float> (fsBase) * 0.040f)));
    meterOut_ = vuState_;
}
}
