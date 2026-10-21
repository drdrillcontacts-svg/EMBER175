#pragma once

#include <JuceHeader.h>

#ifndef JucePlugin_Name
 #define JucePlugin_Name "EMBER 175-B"
#endif
#include "EmberDSP.h"
#include "EmberModel.h"

class Ember175AudioProcessor final : public juce::AudioProcessor
{
public:
    Ember175AudioProcessor();
    ~Ember175AudioProcessor() override = default;

    void prepareToPlay (double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;

    float getGainReductionDb() const noexcept   { return dsp.gainReductionDb(); }
    float getOutputLevelDb() const noexcept     { return dsp.outputLevelDb(); }
    int   getLatencySamples() const noexcept    { return dsp.latencySamples(); }

    // UI-facing helpers (read-only, called from the editor)
    float read (const char* id) const noexcept
    {
        if (auto* v = parameters.getRawParameterValue (id))
            return v->load();
        return 0.0f;
    }

private:
    static APVTS::ParameterLayout createParameterLayout();

    ember::Processor dsp;
    std::atomic<float> grMeter { 0.0f }, outMeter { -100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ember175AudioProcessor)
};
