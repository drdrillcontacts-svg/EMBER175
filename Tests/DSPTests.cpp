/*
    EMBER 175-B - DSP regression and calibration tests.

    These are not "does it sound good" tests. They check the invariants that the
    published 175-B specification pins down (12:1 at the knee, 100 us .. 1 ms
    attack, 27 .. 527 ms release, OFF position, unity gain at rest) and they
    guard the safety properties (no NaN, no runaway, sane aliasing).

    Run with `ember_dsp_tests`, optionally `ember_dsp_tests -v` for the
    measurement table.
*/

#include "EmberDSP.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <chrono>
#include <vector>

namespace
{
constexpr double kPi = 3.14159265358979323846;
int failures = 0;
bool verbose = false;

void check (bool condition, const std::string& what)
{
    if (! condition)
    {
        std::printf ("  FAIL  %s\n", what.c_str());
        ++failures;
    }
    else if (verbose)
    {
        std::printf ("  ok    %s\n", what.c_str());
    }
}

void checkNear (double actual, double expected, double tolerance, const std::string& what)
{
    const bool ok = std::abs (actual - expected) <= tolerance;
    if (! ok)
        std::printf ("  FAIL  %s (got %.3f, expected %.3f +/- %.3f)\n", what.c_str(), actual, expected, tolerance);
    else if (verbose)
        std::printf ("  ok    %s (%.3f)\n", what.c_str(), actual);
    if (! ok)
        ++failures;
}

double rms (const std::vector<float>& x, size_t from = 0)
{
    double sum = 0.0;
    for (size_t i = from; i < x.size(); ++i)
        sum += static_cast<double> (x[i]) * x[i];
    return std::sqrt (sum / static_cast<double> (std::max<size_t> (1, x.size() - from)));
}

double dbfs (double linear)
{
    return 20.0 * std::log10 (linear > 1.0e-12 ? linear : 1.0e-12);
}

// Hann windowed single bin DFT, so that spectral leakage from a loud
// fundamental cannot be mistaken for an alias.
double goertzel (const std::vector<float>& x, double freq, double sr, size_t from = 0)
{
    const size_t count = x.size() - from;
    double re = 0.0, im = 0.0, norm = 0.0;
    for (size_t i = 0; i < count; ++i)
    {
        const double w = 0.5 - 0.5 * std::cos (2.0 * kPi * static_cast<double> (i) / static_cast<double> (count - 1));
        const double phase = 2.0 * kPi * freq * static_cast<double> (i) / sr;
        re += w * x[from + i] * std::cos (phase);
        im += w * x[from + i] * std::sin (phase);
        norm += w;
    }
    return 2.0 * std::sqrt (re * re + im * im) / norm;
}

struct Rig
{
    ember::Processor proc;
    ember::Parameters p;
    double sr = 48000.0;
    int block = 64;

    explicit Rig (double sampleRate = 48000.0, int blockSize = 64, int channels = 2)
        : sr (sampleRate), block (blockSize)
    {
        proc.prepare (sr, block, channels);
    }

    // Process a mono tone, returns the left channel.
    std::vector<float> tone (double freq, double levelDbfs, double seconds, double silenceAfter = 0.0,
                             float* grTrace = nullptr, int grTraceStride = 0, int* grCount = nullptr)
    {
        const int total = static_cast<int> (sr * seconds);
        const int tail = static_cast<int> (sr * silenceAfter);
        std::vector<float> out (static_cast<size_t> (total + tail), 0.0f);
        std::vector<float> work (static_cast<size_t> (block), 0.0f);
        const float amp = static_cast<float> (std::pow (10.0, levelDbfs / 20.0));
        int written = 0;
        int traceIndex = 0;

        while (written < total + tail)
        {
            const int n = std::min (block, total + tail - written);
            for (int i = 0; i < n; ++i)
            {
                const int idx = written + i;
                const double phase = 2.0 * kPi * freq * static_cast<double> (idx) / sr;
                work[static_cast<size_t> (i)] = idx < total ? static_cast<float> (amp * std::sin (phase)) : 0.0f;
            }
            float* ptrs[1] { work.data() };
            proc.process (ptrs, 1, n, p);
            std::memcpy (out.data() + written, work.data(), sizeof (float) * static_cast<size_t> (n));

            if (grTrace != nullptr && grTraceStride > 0 && (written / grTraceStride) != ((written + n) / grTraceStride))
            {
                if (grCount != nullptr && traceIndex < *grCount)
                    grTrace[traceIndex++] = proc.instantaneousGainReductionDb();
            }
            written += n;
        }
        if (grCount != nullptr)
            *grCount = traceIndex;
        return out;
    }
};
}

int main (int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
        if (std::strcmp (argv[i], "-v") == 0 || std::strcmp (argv[i], "--verbose") == 0)
            verbose = true;

    std::printf ("EMBER 175-B DSP tests\n");
    std::printf ("=====================\n");

    // ---------------------------------------------------------------- 1 bypass
    {
        std::printf ("\n[1] bypass / latency / transparency\n");
        ember::Processor proc;
        proc.prepare (48000.0, 256, 2);
        const int latency = proc.latencySamples();
        check (latency > 0 && latency < 200, "reported latency is sane");

        std::vector<float> in (8192), out (8192);
        for (size_t i = 0; i < in.size(); ++i)
            in[i] = static_cast<float> (0.3 * std::sin (2.0 * kPi * 441.0 * i / 48000.0));

        out = in;
        ember::Parameters p;
        p.bypass = true;
        float* ptrs[1] { out.data() };
        proc.process (ptrs, 1, static_cast<int> (out.size()), p);

        bool transparent = true;
        for (size_t i = 0; i < in.size(); ++i)
        {
            const size_t src = i >= static_cast<size_t> (latency) ? i - static_cast<size_t> (latency) : 0;
            if (i >= static_cast<size_t> (latency) && out[i] != in[src])
                transparent = false;
        }
        check (transparent, "bypass is bit transparent (delayed by the reported latency)");

        // the reported latency must equal the real delay of the processed path,
        // otherwise the dry/wet mix would comb and the host would mis-align
        {
            ember::Processor wet;
            wet.prepare (48000.0, 256, 1);
            std::vector<float> impulse (512, 0.0f);
            impulse[64] = 0.5f;
            ember::Parameters q;
            q.inputDb = 20.0f;
            q.outputDb = 20.0f;
            q.attackMs = 0.0f;              // no limiting: measure the chain only
            float* ptrs[1] { impulse.data() };
            wet.process (ptrs, 1, static_cast<int> (impulse.size()), q);

            int peak = 0;
            float peakValue = 0.0f;
            for (int i = 0; i < static_cast<int> (impulse.size()); ++i)
                if (std::abs (impulse[static_cast<size_t> (i)]) > std::abs (peakValue))
                {
                    peakValue = impulse[static_cast<size_t> (i)];
                    peak = i;
                }
            checkNear (peak - 64, wet.latencySamples(), 0.51,
                       "reported latency matches the measured delay of the chain");
        }

        // unity gain with no limiting: a quiet tone must pass untouched
        proc.reset();
        p.bypass = false;
        p.inputDb = 20.0f;
        p.outputDb = 20.0f;
        p.attackMs = 0.3f;
        p.releaseMs = 120.0f;
        Rig rig;
        rig.p = p;
        auto quiet = rig.tone (1000.0, -34.0, 1.0);
        const double lvl = dbfs (rms (quiet, quiet.size() / 2));
        checkNear (lvl, -37.0, 0.6, "unity gain on a quiet -34 dBFS peak tone");
        check (rig.proc.instantaneousGainReductionDb() < 0.05f, "no gain reduction below threshold");
    }

    // ------------------------------------------------------------ 2 static ratio
    {
        std::printf ("\n[2] static transfer / 12:1 ratio\n");
        ember::Parameters p;
        p.inputDb = 20.0f;      // unity drive: the knee sits inside the sweep
        p.outputDb = 20.0f;
        p.attackMs = 0.1f;
        p.releaseMs = 27.0f;

        std::vector<double> inDb, outDb;
        for (double drive = -12.0; drive <= 12.0; drive += 2.0)
        {
            Rig rig (48000.0, 64);
            rig.p = p;
            auto y = rig.tone (997.0, static_cast<float> (drive), 1.2);
            inDb.push_back (drive);
            double peak = 0.0;
            for (size_t i = y.size() * 3 / 4; i < y.size(); ++i)
                peak = std::max (peak, static_cast<double> (std::abs (y[i])));
            outDb.push_back (dbfs (peak));
            if (verbose)
                std::printf ("      in %+6.1f dBFS  ->  out %+7.2f dBFS   GR %6.2f dB\n",
                             drive, outDb.back(), drive - outDb.back());
        }

        // slope between 4 dB and 8 dB input: 1/ratio
        // find the knee (first step where gain reduction appears) and measure the
        // ratio just above it: that is what the hardware spec calls 12:1
        size_t knee = 1;
        for (size_t i = 1; i < inDb.size(); ++i)
            if (inDb[i] - outDb[i] >= 0.5)
            {
                knee = i;
                break;
            }
        double slope = 0.0;
        for (size_t i = knee; i < inDb.size() && i <= knee + 2; ++i)
            slope += (outDb[i] - outDb[i - 1]) / (inDb[i] - inDb[i - 1]);
        slope /= static_cast<double> (std::min<size_t> (3, inDb.size() - knee));
        const double ratio = 1.0 / std::max (1.0e-6, slope);
        check (ratio > 9.0 && ratio < 16.0, "ratio is 12:1 just above threshold");
        if (verbose)
            std::printf ("      measured ratio near threshold: %.1f:1\n", ratio);

        // hard driven: output must stay pinned near the threshold
        check (outDb.back() < -4.0 && outDb.back() > -9.0, "output stays pinned near the threshold under hard drive");

        // the 175-B gets steeper as it is driven harder
        double slopeHigh = 0.0;
        int highCount = 0;
        for (size_t i = inDb.size() - 4; i < inDb.size(); ++i)
        {
            slopeHigh += (outDb[i] - outDb[i - 1]) / (inDb[i] - inDb[i - 1]);
            ++highCount;
        }
        slopeHigh /= std::max (1, highCount);
        if (verbose)
            std::printf ("      measured ratio at high drive: %.1f:1\n",
                         slopeHigh > 1.0e-4 ? 1.0 / slopeHigh : 9999.0);
        check (slopeHigh < slope, "ratio stiffens as the unit is driven harder");

        // output must never exceed the input for a hard driven signal
        check (outDb.back() < inDb.back(), "output is below input when driven");
    }

    // --------------------------------------------------------- 3 attack / release
    {
        std::printf ("\n[3] attack and release ballistics\n");

        auto measureAttack = [] (float attackMs)
        {
            ember::Parameters p;
            p.inputDb = 30.0f;
            p.outputDb = 20.0f;
            p.attackMs = attackMs;
            p.releaseMs = 200.0f;
            Rig rig (48000.0, 2);
            rig.p = p;
            std::vector<float> trace (6000, 0.0f);
            int count = static_cast<int> (trace.size());
            rig.tone (5000.0, -3.0, 0.05, 0.0, trace.data(), 1, &count);

            float finalGr = 0.0f;
            for (int i = count - 8; i < count; ++i)
                finalGr = std::max (finalGr, trace[static_cast<size_t> (i)]);
            const float target = finalGr * 0.632f;
            int idx = -1;
            for (int i = 0; i < count; ++i)
                if (trace[static_cast<size_t> (i)] >= target) { idx = i; break; }
            // the trace advances one entry per processed block (2 samples); the
            // sidechain also sees the interpolated (delayed) signal, so take
            // that out to report the true attack time
            const double scMs = rig.proc.sidechainDelaySamples() * 1000.0 / 48000.0;
            return std::make_pair (idx < 0 ? -1.0 : idx * 2.0 / 48000.0 * 1000.0 - scMs, static_cast<double> (finalGr));
        };

        const auto fast = measureAttack (0.1f);
        const auto slow = measureAttack (1.0f);
        if (verbose)
            std::printf ("      attack 0.10 ms -> measured %.3f ms (GR %.1f dB)\n", fast.first, fast.second);
        if (verbose)
            std::printf ("      attack 1.00 ms -> measured %.3f ms (GR %.1f dB)\n", slow.first, slow.second);

        check (fast.second > 2.0, "fast attack produces meaningful gain reduction");
        check (fast.first > 0.03 && fast.first < 0.40, "100 us attack lands in a plausible window");
        check (slow.first > fast.first * 1.5, "1 ms attack is clearly slower than 100 us");
        check (slow.first > 0.4 && slow.first < 4.0, "1 ms attack lands in a plausible window");

        // release: 63 % recovery time, measured after the tone stops
        auto measureRelease = [] (float releaseMs)
        {
            ember::Parameters p;
            p.inputDb = 30.0f;
            p.attackMs = 0.1f;
            p.releaseMs = releaseMs;
            Rig rig (48000.0, 16);
            rig.p = p;
            // 0.5 s tone, then 1.2 s silence, sample the GR every block
            std::vector<float> trace (4200, 0.0f);
            int count = 0;
            const float amp = static_cast<float> (std::pow (10.0, -3.0 / 20.0));
            std::vector<float> work (16);
            int idx = 0;
            double peakGr = 0.0;
            int targetIdx = -1;
            while (count < 4200)
            {
                for (int i = 0; i < 16; ++i, ++idx)
                {
                    const bool on = idx < static_cast<int> (48000 * 0.5);
                    work[static_cast<size_t> (i)] = on ? static_cast<float> (amp * std::sin (2.0 * kPi * 997.0 * idx / 48000.0)) : 0.0f;
                }
                float* ptrs[1] { work.data() };
                rig.proc.process (ptrs, 1, 16, p);
                const float gr = rig.proc.instantaneousGainReductionDb();
                trace[static_cast<size_t> (count++)] = gr;
                if (idx > static_cast<int> (48000 * 0.45))
                    peakGr = std::max (peakGr, static_cast<double> (gr));
            }
            // find where the tone ends
            int endIdx = static_cast<int> (48000 * 0.5 / 16);
            const double target = static_cast<double> (trace[static_cast<size_t> (endIdx)]) * 0.37;
            for (int i = endIdx; i < count; ++i)
                if (trace[static_cast<size_t> (i)] <= target) { targetIdx = i; break; }
            const double ms = targetIdx < 0 ? -1.0 : (targetIdx - endIdx) * 16.0 / 48000.0 * 1000.0;
            return std::make_pair (ms, static_cast<double> (trace[static_cast<size_t> (endIdx)]));
        };

        const auto relFast = measureRelease (27.0f);
        const auto relSlow = measureRelease (527.0f);
        if (verbose)
            std::printf ("      release  27 ms -> measured %.0f ms (GR at cut %.1f dB)\n",
                         std::max (0.0, relFast.first), relFast.second);
        if (verbose)
            std::printf ("      release 527 ms -> measured %.0f ms (GR at cut %.1f dB)\n", relSlow.first, relSlow.second);

        check (relFast.first > 5.0 && relFast.first < 150.0, "27 ms release is quick but not instant");
        check (relSlow.first > relFast.first * 2.0, "527 ms release is clearly slower");
        check (relSlow.first < 2000.0, "527 ms release does not hang");
    }

    // ------------------------------------------------------------- 4 attack OFF
    {
        std::printf ("\n[4] attack OFF = tone box, no gain reduction\n");
        ember::Parameters p;
        p.inputDb = 36.0f;
        p.attackMs = 0.0f;
        p.releaseMs = 120.0f;
        Rig rig (48000.0, 64);
        rig.p = p;
        auto y = rig.tone (997.0, -6.0, 0.6);
        check (rig.proc.instantaneousGainReductionDb() < 0.01f, "limiter disabled, path still passes audio");
        check (rms (y, y.size() / 2) > 0.05, "tone box output is not silent");
    }

    // ---------------------------------------------------------------- 5 mix
    {
        std::printf ("\n[5] dry / wet\n");
        ember::Parameters p;
        p.inputDb = 34.0f;
        p.attackMs = 0.1f;
        p.releaseMs = 50.0f;
        p.mix = 0.0f;
        Rig rig (48000.0, 64);
        rig.p = p;
        auto y = rig.tone (997.0, -6.0, 0.5);
        const double lvl = dbfs (rms (y, y.size() / 2));
        checkNear (lvl, -9.0, 0.15, "mix at 0 % is exactly the dry signal");

        p.mix = 1.0f;
        rig.p = p;
        auto wet = rig.tone (997.0, -6.0, 0.5);
        check (rig.proc.instantaneousGainReductionDb() > 3.0f, "mix at 100 % compresses");
        check (dbfs (rms (wet, wet.size() / 2)) > lvl - 1.0, "wet path is not silent");
    }

    // -------------------------------------------------------------- 6 stereo link
    {
        std::printf ("\n[6] stereo link\n");
        auto run = [] (bool link)
        {
            ember::Processor proc;
            proc.prepare (48000.0, 64, 2);
            ember::Parameters q;
            q.inputDb = 30.0f;
            q.attackMs = 0.1f;
            q.releaseMs = 40.0f;
            q.stereoLink = link;
            std::vector<float> l (9600), r (9600);
            for (size_t i = 0; i < l.size(); ++i)
            {
                l[i] = static_cast<float> (0.5 * std::sin (2.0 * kPi * 220.0 * i / 48000.0));
                r[i] = static_cast<float> (0.01 * std::sin (2.0 * kPi * 330.0 * i / 48000.0));
            }
            for (int b = 0; b < 9600; b += 64)
            {
                float* ptrs[2] { l.data() + b, r.data() + b };
                proc.process (ptrs, 2, 64, q);
            }
            return std::make_pair (dbfs (rms (l, 4800)), dbfs (rms (r, 4800)));
        };

        const auto linked = run (true);
        const auto split = run (false);
        check (split.second > linked.second + 2.5, "linked: the quiet channel is pulled down by the loud one");
        check (split.second > -50.0, "unlinked: quiet channel keeps its level");
    }

    // ----------------------------------------------------------- 7 aliasing
    {
        std::printf ("\n[7] oversampling / aliasing\n");
        ember::Parameters p;
        p.inputDb = 34.0f;
        p.outputDb = 20.0f;
        p.attackMs = 0.1f;
        p.releaseMs = 27.0f;
        Rig rig (48000.0, 64);
        rig.p = p;
        auto y = rig.tone (11000.0, -6.0, 0.5);
        const double fund = goertzel (y, 11000.0, 48000.0, 12000);
        const double alias3 = goertzel (y, 15000.0, 48000.0, 12000);   // 3rd harmonic folded
        const double alias5 = goertzel (y, 7000.0, 48000.0, 12000);    // 5th harmonic folded
        const double rel = std::max (alias3, alias5) / std::max (fund, 1.0e-9);
        if (verbose)
            std::printf ("      worst folded harmonic: %.1f dB re fundamental\n", 20.0 * std::log10 (rel));
        check (rel < 0.01, "folded harmonics below -40 dB relative to the fundamental");
    }

    // ------------------------------------------------------- 8 stability sweep
    {
        std::printf ("\n[8] stability across sample rates and abuse\n");
        for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
        {
            ember::Processor proc;
            proc.prepare (sr, 512, 2);
            ember::Parameters p;
            p.inputDb = 40.0f;
            p.outputDb = 40.0f;
            p.attackMs = 0.1f;
            p.releaseMs = 27.0f;
            p.grTrimDb = 6.0f;

            std::vector<float> l (4096), r (4096);
            for (size_t i = 0; i < l.size(); ++i)
            {
                // square wave at full scale, the worst case for any limiter
                l[i] = (std::fmod (static_cast<double> (i) * 997.0 / sr, 1.0) < 0.5) ? 1.0f : -1.0f;
                r[i] = l[i];
            }
            float* ptrs[2] { l.data(), r.data() };
            bool finite = true;
            for (int b = 0; b < 4096; b += 512)
            {
                proc.process (ptrs, 2, 512, p);
                for (int i = 0; i < 4096; ++i)
                    if (! std::isfinite (l[static_cast<size_t> (i)]) || ! std::isfinite (r[static_cast<size_t> (i)]))
                        finite = false;
            }
            check (finite, "finite output at " + std::to_string (static_cast<int> (sr)) + " Hz with full scale square input");
            check (std::abs (l[4095]) < 4.0f, "bounded output at " + std::to_string (static_cast<int> (sr)) + " Hz");
        }
    }

    // ------------------------------------------------------------ 9 parameter sanity
    {
        std::printf ("\n[9] parameter ranges\n");
        check (ember::model::attackNormToMs (0.0f) == 0.0f, "attack at minimum is the OFF position");
        check (ember::model::attackNormToMs (1.0f) > 0.09f && ember::model::attackNormToMs (1.0f) < 0.11f,
               "attack fully clockwise is 100 us");
        const float mid = ember::model::attackMsToNorm (0.3f);
        check (ember::model::attackNormToMs (mid) > 0.28f && ember::model::attackNormToMs (mid) < 0.32f,
               "attack taper round trips");
        check (ember::model::releaseFromNorm (0.0f) == 27.0f, "release fully anticlockwise is 27 ms");
        check (ember::model::releaseFromNorm (1.0f) == 527.0f, "release fully clockwise is 527 ms");
    }

    // ----------------------------------------------------------- 10 performance
    {
        std::printf ("\n[10] throughput\n");
        ember::Processor proc;
        proc.prepare (48000.0, 512, 2);
        ember::Parameters p;
        p.inputDb = 28.0f;
        std::vector<float> l (512), r (512);
        for (size_t i = 0; i < 512; ++i)
            l[i] = r[i] = static_cast<float> (0.2 * std::sin (i * 0.05));
        float* ptrs[2] { l.data(), r.data() };
        const int blocks = 4000;   // ~ 42 s of audio
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < blocks; ++i)
            proc.process (ptrs, 2, 512, p);
        const double seconds = std::chrono::duration<double> (std::chrono::steady_clock::now() - start).count();
        const double audioSeconds = blocks * 512.0 / 48000.0;
        const double realtimeFactor = audioSeconds / seconds;
        std::printf ("      %.1f x realtime on one core (stereo, 48 kHz, 4x oversampled)\n", realtimeFactor);
        check (realtimeFactor > 3.0, "engine is comfortably faster than realtime");
    }

    std::printf ("\n%s (%d failure%s)\n", failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED",
                 failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
