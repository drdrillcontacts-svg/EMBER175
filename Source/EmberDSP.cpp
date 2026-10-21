#include "EmberDSP.h"

namespace ember
{
float Processor::dbToGain(float db) noexcept
{
    return std::pow(10.0f, db / 20.0f);
}

float Processor::gainToDb(float gain) noexcept
{
    return 20.0f * std::log10(std::max(gain, 1.0e-9f));
}

float Processor::smoothCoefficient(double sampleRate, float timeMs) noexcept
{
    const float seconds = std::max(0.0001f, timeMs * 0.001f);
    return std::exp(-1.0f / (static_cast<float>(sampleRate) * seconds));
}

float Processor::tubeColor(float x, float amount) noexcept
{
    // Deliberately simple, bounded nonlinear stage for research; not a tube-circuit model.
    const float a = std::clamp(amount, 0.0f, 1.0f);
    if (a <= 1.0e-5f)
        return x;
    const float drive = 1.0f + 4.0f * a;
    const float shaped = std::tanh(x * drive) / std::tanh(drive);
    return x + (shaped - x) * a;
}

void Processor::prepare(double sampleRate, int maxBlockSize, int channels)
{
    (void) maxBlockSize;
    sampleRate_ = std::max(8000.0, sampleRate);
    maxChannels_ = std::clamp(channels, 1, 2);
    reset();
}

void Processor::reset()
{
    envelope_ = 0.0f;
    gain_ = 1.0f;
    gainReductionDb_ = 0.0f;
    dcState_[0] = dcState_[1] = 0.0f;
    detectorAttack_ = smoothCoefficient(sampleRate_, 2.0f);
    detectorRelease_ = smoothCoefficient(sampleRate_, 100.0f);
    gainAttack_ = smoothCoefficient(sampleRate_, 1.0f);
    gainRelease_ = smoothCoefficient(sampleRate_, 80.0f);
}

void Processor::process(float* const* channels, int numChannels, int numSamples, const Parameters& p)
{
    if (channels == nullptr || numSamples <= 0)
        return;

    const int count = std::clamp(numChannels, 1, maxChannels_);
    const float inputGain = dbToGain(p.inputDb - p.headroomDb);
    const float outputGain = dbToGain(p.outputDb + p.headroomDb);
    const float threshold = p.thresholdDb;
    const float ratio = 12.0f; // 175B-style fixed high ratio; intentionally not user-selectable.
    const float mix = std::clamp(p.mix, 0.0f, 1.0f);
    const float color = std::clamp(p.color, 0.0f, 1.0f);

    detectorAttack_ = smoothCoefficient(sampleRate_, std::clamp(p.attackMs, 0.1f, 100.0f));
    detectorRelease_ = smoothCoefficient(sampleRate_, std::clamp(p.releaseMs, 5.0f, 2000.0f));
    gainAttack_ = smoothCoefficient(sampleRate_, std::clamp(p.attackMs * 0.5f, 0.05f, 100.0f));
    gainRelease_ = smoothCoefficient(sampleRate_, std::clamp(p.releaseMs, 5.0f, 2000.0f));

    if (p.bypass)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float linkedDetector = 0.0f;
        float currentInput[2] { 0.0f, 0.0f };
        float previousOutputPeak = 0.0f;

        for (int ch = 0; ch < count; ++ch)
        {
            currentInput[ch] = channels[ch][i] * inputGain;
            const float detectorSample = std::abs(currentInput[ch] * gain_);
            linkedDetector = std::max(linkedDetector, detectorSample);
            previousOutputPeak = std::max(previousOutputPeak, std::abs(currentInput[ch] * gain_));
        }

        // Feedback-inspired detector: detector follows the compressed signal estimate.
        // A measured reference is required to tune this approximation to a specific unit.
        const float coeff = linkedDetector > envelope_ ? detectorAttack_ : detectorRelease_;
        envelope_ = coeff * envelope_ + (1.0f - coeff) * linkedDetector;

        const float levelDb = gainToDb(envelope_);
        const float overDb = std::max(0.0f, levelDb - threshold);
        const float targetReductionDb = overDb * (1.0f - 1.0f / ratio);
        const float targetGain = dbToGain(-targetReductionDb);
        const float gainCoeff = targetGain < gain_ ? gainAttack_ : gainRelease_;
        gain_ = gainCoeff * gain_ + (1.0f - gainCoeff) * targetGain;
        gainReductionDb_ = gainToDb(gain_);

        for (int ch = 0; ch < count; ++ch)
        {
            const float dry = channels[ch][i];
            const float compressed = currentInput[ch] * gain_;
            const float coloured = tubeColor(compressed, color);
            channels[ch][i] = (dry * (1.0f - mix) + coloured * mix) * outputGain;
        }
    }
}
}
