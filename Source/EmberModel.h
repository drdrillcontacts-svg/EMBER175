#pragma once

/*
    EMBER 175-B - shared model constants.

    All calibration values that describe the reference behaviour of the
    limiting amplifier live here so that the DSP and the user interface agree
    on ranges, taper and meter scaling.

    Reference (published UA 175-B specification, used for calibration only):
      - fixed 12:1 compression ratio at the threshold of limiting, with the
        ratio increasing above threshold (feedback / vari-mu topology)
      - attack      100 us .. 1000 us, clockwise = faster, minimum = OFF
      - release      27 ms ..  527 ms, clockwise = faster
      - input/output attenuators 0 .. 40 dB in 2 dB steps plus a +/-2 dB vernier
      - minimum input level to limit  -24 dBm, max output at threshold +13 dBm
      - signal to noise ratio 80 dB
*/

#include <cmath>

namespace ember
{
namespace model
{
    // ------------------------------------------------------------------ ranges
    constexpr float kInputMinDb      = 0.0f;
    constexpr float kInputMaxDb      = 40.0f;
    constexpr float kOutputMinDb     = 0.0f;
    constexpr float kOutputMaxDb     = 40.0f;
    constexpr float kVernierDb       = 5.0f;    // vernier trim, printed +/- 5 dB
    constexpr float kGrVernierDb     = 6.0f;    // G/R vernier: threshold calibration
    constexpr float kAttackMinMs     = 0.1f;
    constexpr float kAttackMaxMs     = 1.0f;
    constexpr float kAttackOffMs     = 0.0f;    // attack knob fully anticlockwise = OFF
    constexpr float kReleaseMinMs    = 27.0f;
    constexpr float kReleaseMaxMs    = 527.0f;
    constexpr float kHeadroomMinDb   = -12.0f;
    constexpr float kHeadroomMaxDb   = 12.0f;
    constexpr float kStepDb          = 2.0f;    // stepped attenuator detents

    // ------------------------------------------------------------- calibration
    // The attenuator network on the hardware sits in front of a ~37 dB amplifier;
    // we normalise that to a 20 dB reference so that 20 / 20 is unity gain.
    constexpr float kAttenuatorReferenceDb = 20.0f;

    // Level at the sidechain tap (post vari-mu stage, post output amplifier,
    // before the output attenuator) at which limiting starts, in dBFS.
    constexpr float kThresholdDbfs = -6.2f;

    // Fixed 12:1 at the threshold of limiting. The ratio then stiffens with the
    // amount of over drive: 24:1 at 10 dB over, 36:1 at 20 dB over, which is
    // the "brick wall when hit hard" behaviour the 175-B is known for.
    constexpr float kRatioAtKnee = 12.0f;
    constexpr float kRatioGrowth = 0.40f;

    inline float ratioForOverdrive (float overDb) noexcept
    {
        return kRatioAtKnee + kRatioGrowth * (overDb > 0.0f ? overDb : 0.0f);
    }

    constexpr float kMaxGainReductionDb = 46.0f;
    constexpr float kKneeDb = 0.6f;             // 175-B knee is famously hard

    // ------------------------------------------------------------- metering
    constexpr float kMeterGrMaxDb = 20.0f;      // GR meter swing
    constexpr float kMeterVuMinDb = -20.0f;     // VU scale, 0 VU = 0 dBFS

    // ------------------------------------------------------------- utilities
    inline float dbToGain (float db) noexcept
    {
        return std::pow (10.0f, db * 0.05f);
    }

    inline float gainToDb (float gain) noexcept
    {
        return 20.0f * std::log10 (gain > 1.0e-9f ? gain : 1.0e-9f);
    }

    // Both time knobs read left to right as slower: the dials are printed
    // ".1 .3 1" and "30 120 500", and the attack knob has an OFF detent at the
    // extreme anticlockwise position.
    constexpr float kAttackOffNorm = 0.06f;

    inline float attackNormToMs (float normalised) noexcept
    {
        const float n = normalised < 0.0f ? 0.0f : (normalised > 1.0f ? 1.0f : normalised);
        if (n <= kAttackOffNorm)
            return kAttackOffMs;
        const float t = (n - kAttackOffNorm) / (1.0f - kAttackOffNorm);
        return kAttackMinMs * std::pow (kAttackMaxMs / kAttackMinMs, t);
    }

    inline float attackMsToNorm (float ms) noexcept
    {
        if (ms <= kAttackOffMs)
            return 0.0f;
        const float t = ms < kAttackMinMs ? kAttackMinMs : (ms > kAttackMaxMs ? kAttackMaxMs : ms);
        const float n = std::log (t / kAttackMinMs) / std::log (kAttackMaxMs / kAttackMinMs);
        return kAttackOffNorm + n * (1.0f - kAttackOffNorm);
    }

    inline float releaseFromNorm (float normalised) noexcept
    {
        const float n = normalised < 0.0f ? 0.0f : (normalised > 1.0f ? 1.0f : normalised);
        return kReleaseMinMs * std::pow (kReleaseMaxMs / kReleaseMinMs, n);
    }

    inline float releaseToNorm (float ms) noexcept
    {
        const float t = ms < kReleaseMinMs ? kReleaseMinMs
                                           : (ms > kReleaseMaxMs ? kReleaseMaxMs : ms);
        return std::log (t / kReleaseMinMs) / std::log (kReleaseMaxMs / kReleaseMinMs);
    }
}
}
