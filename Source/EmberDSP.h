#pragma once

/*
    EMBER 175-B - DSP engine (no JUCE dependency, unit testable).

    Topology (as far as it is publicly documented for the 175-B):

        input attenuator
          -> input transformer   (UTC, LF core saturation, 2nd order asymmetry)
          -> 6BC8 vari-mu stage  (feedback gain reduction, gain law from a
                                  remote-cutoff dual triode)
          -> 12AX7 / 12BH7 amp   (gentle single ended drive, DC blocking)
          -> output level        (attenuator, after the sidechain tap)

    The sidechain is a 6AL5 full wave rectifier feeding an attack/release
    network whose output is a control voltage for the 6BC8 grids. Because the
    detector senses the *output* of the gain stage, the loop closes on itself:
    that is what produces the characteristic "soft at low level, brick wall at
    high level" 12:1 behaviour of the 175-B.

    Everything runs 4x oversampled around the non-linear stages.
*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "EmberModel.h"

namespace ember
{
struct Parameters
{
    float inputDb      = 20.0f;   // 0 .. 40   input attenuator / threshold
    float inputTrimDb  = 0.0f;    // +/- 2     input vernier
    float outputDb     = 20.0f;   // 0 .. 40   output attenuator
    float outputTrimDb = 0.0f;    // +/- 2     output vernier
    float attackMs     = 0.3f;    // 0.1 .. 1.0, 0 = OFF
    float releaseMs    = 120.0f;  // 27 .. 527
    float grTrimDb     = 0.0f;    // +/- 6     G/R vernier (threshold calibration)
    float headroomDb   = 0.0f;    // -12 .. 12
    float mix          = 1.0f;    // 0 .. 1
    bool  bypass       = false;
    bool  stereoLink   = true;
    bool  detectorHighPass = false;   // LOW / HI switch: HI high-passes the sidechain
    bool  lowZInput    = false;       // 300 ohm position: heavier transformer loading
};

class Processor
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void process (float* const* channels, int numChannels, int numSamples, const Parameters& p);

    float gainReductionDb() const noexcept  { return meterGr_; }
    float instantaneousGainReductionDb() const noexcept { return instantGr_; }
    float outputLevelDb() const noexcept    { return meterOut_; }
    bool  isLimiting() const noexcept       { return meterGr_ > 0.05f; }
    int   latencySamples() const noexcept   { return latency_; }
    int   sidechainDelaySamples() const noexcept { return sidechainDelay_; }

private:
    static constexpr int kMaxChannels = 2;
    static constexpr int kHalfBandTaps = 63;
    static constexpr int kHalfBandRing = 64;    // power of two history length
    static constexpr int kHalfBandCentre = (kHalfBandTaps - 1) / 2;
    static constexpr int kHalfBandPhase = (kHalfBandTaps + 1) / 2;

    struct HalfBand
    {
        void design();
        void processUp (const float* in, float* out, int n) noexcept;
        void processDown (const float* in, float* out, int n) noexcept;
        void reset() noexcept;

        float h[kHalfBandTaps] {};
        float even[kHalfBandPhase] {};      // non zero taps, phase 0
        float hist[kHalfBandRing] {};       // upsample / downsample history
        int write = 0;
    };

    struct ChannelState
    {
        float xfHpZ = 0.0f;      // input transformer coupling
        float lfZ = 0.0f;        // low frequency band for core saturation
        float lfSatZ = 0.0f;     // LF saturation memory
        float dcZ = 0.0f;        // DC block after the vari-mu stage
        float ampZ = 0.0f;       // amplifier coupling
        float outZ = 0.0f;       // output transformer coupling
        float scHpZ = 0.0f;      // sidechain high-pass memory
        float env = 0.0f;        // 6AL5 rectifier memory
        float cv1 = 0.0f;        // attack capacitor, dB domain
        float cv2 = 0.0f;        // release tail capacitor, dB domain
        float grDb = 0.0f;       // current gain reduction, dB
        float gain = 1.0f;       // current 6BC8 gain
    };

    void updateRates (const Parameters& p) noexcept;
    void processChunk (float* const* channels, int numChannels, int numSamples, const Parameters& p);

    double sampleRate_ = 48000.0;
    int maxBlockSize_ = 512;
    int numChannels_ = 2;
    int stages_ = 2;              // 2 -> 4x, 1 -> 2x
    int latency_ = 0;
    int sidechainDelay_ = 0;

    HalfBand up_[kMaxChannels][2];
    HalfBand down_[kMaxChannels][2];
    ChannelState state_[kMaxChannels];

    std::vector<float> osCh_[kMaxChannels];
    std::vector<float> scratch_[kMaxChannels];
    std::vector<float> dryBuf_[kMaxChannels];
    int dryWrite_ = 0;

    // smoothed parameters
    float inGain_ = 1.0f, outGain_ = 1.0f, mix_ = 1.0f, charPre_ = 1.0f, charPost_ = 1.0f;
    float attackCoef_ = 0.0f, relFastCoef_ = 0.0f, relSlowCoef_ = 0.0f;
    float rectCoef_ = 0.0f, rectFastCoef_ = 0.0f, scHpCoef_ = 0.0f;
    float ironDrive_ = 1.0f, ironComp_ = 1.0f;
    float thresholdDb_ = -6.2f, ratioKnee_ = 12.0f, ratioGrowth_ = 1.2f, maxGr_ = 46.0f;
    bool  limiting_ = true;
    bool  linked_ = true;
    bool  snapParams_ = true;
    bool  detectorHighPass_ = false;

    // meters
    float instantGr_ = 0.0f;
    float meterGr_ = 0.0f;
    float meterOut_ = -100.0f;
    float vuState_ = 0.0f;

    static inline float flushDenormal (float x) noexcept
    {
        return std::abs (x) < 1.0e-25f ? 0.0f : x;
    }
};
}
