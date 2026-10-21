#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ember
{
struct Parameters
{
    float inputDb = 0.0f;
    float outputDb = 0.0f;
    float thresholdDb = -18.0f;
    float attackMs = 2.0f;
    float releaseMs = 120.0f;
    float headroomDb = 0.0f;
    float mix = 1.0f;
    float color = 0.15f;
    bool bypass = false;
    bool stereoLink = true;
};

class Processor
{
public:
    void prepare(double sampleRate, int maxBlockSize, int channels);
    void reset();
    void process(float* const* channels, int numChannels, int numSamples, const Parameters& p);
    float envelope() const noexcept { return envelope_; }
    float gainReductionDb() const noexcept { return gainReductionDb_; }

private:
    double sampleRate_ = 44100.0;
    int maxChannels_ = 2;
    float envelope_ = 0.0f;
    float gain_ = 1.0f;
    float gainReductionDb_ = 0.0f;
    float detectorAttack_ = 0.0f;
    float detectorRelease_ = 0.0f;
    float gainAttack_ = 0.0f;
    float gainRelease_ = 0.0f;
    float dcState_[2] { 0.0f, 0.0f };

    static float dbToGain(float db) noexcept;
    static float gainToDb(float gain) noexcept;
    static float smoothCoefficient(double sampleRate, float timeMs) noexcept;
    static float tubeColor(float x, float amount) noexcept;
};
}
