#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    juce::String dbText (float value)
    {
        return (value > 0.0f ? "+" : "") + juce::String (value, 1);
    }
}

Ember175AudioProcessor::Ember175AudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

Ember175AudioProcessor::APVTS::ParameterLayout Ember175AudioProcessor::createParameterLayout()
{
    using R = juce::NormalisableRange<float>;
    APVTS::ParameterLayout l;

    // ---------------------------------------------------------------- front panel
    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "input", 1 }, "Input",
        R (ember::model::kInputMinDb, ember::model::kInputMaxDb, 0.1f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB").withStringFromValueFunction (
            [] (float v, int) { return dbText (v); })));

    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 }, "Output",
        R (ember::model::kOutputMinDb, ember::model::kOutputMaxDb, 0.1f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB").withStringFromValueFunction (
            [] (float v, int) { return dbText (v); })));

    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "attack", 1 }, "Attack",
        R (0.0f, 1.0f, 0.0001f), ember::model::attackMsToNorm (0.3f),
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) {
                const float ms = ember::model::attackNormToMs (v);
                return ms <= 0.0f ? juce::String ("OFF") + " "
                                  : (ms < 0.1f ? juce::String (ms * 1000.0f, 0)
                                               : juce::String (ms, 2)) + " ms";
            })));

    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "release", 1 }, "Release",
        R (0.0f, 1.0f, 0.0001f), ember::model::releaseToNorm (120.0f),
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { return juce::String (ember::model::releaseFromNorm (v), 0) + " ms"; })));

    // input vernier, printed +/- 2 dB around the input knob
    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "vernier", 1 }, "Input Vernier",
        R (-ember::model::kVernierDb, ember::model::kVernierDb, 0.05f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { return dbText (v); })));

    // front panel switches
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "lowHigh", 1 }, "Detector LF Response", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "impedance", 1 }, "Input Impedance 300", false));

    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grTrim", 1 }, "G/R Vernier",
        R (-ember::model::kGrVernierDb, ember::model::kGrVernierDb, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { return dbText (v); })));

    // ------------------------------------------------------------- extended controls
    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Parallel Mix",
        R (0.0f, 100.0f, 0.1f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (
            [] (float v, int) { return juce::String (v, 0) + " %"; })));

    l.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "headroom", 1 }, "Headroom",
        R (ember::model::kHeadroomMinDb, ember::model::kHeadroomMaxDb, 0.5f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction (
            [] (float v, int) { return dbText (v); })));

    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "stereoLink", 1 }, "Stereo Link", true));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "continuous", 1 }, "Continuous Attenuators", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "grMeterMode", 1 }, "Meter Shows Gain Reduction", true));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "power", 1 }, "Power", true));

    return l;
}

void Ember175AudioProcessor::prepareToPlay (double sampleRate, int blockSize)
{
    dsp.prepare (sampleRate, blockSize, juce::jmax (1, getTotalNumOutputChannels()));
    setLatencySamples (dsp.latencySamples());
}

void Ember175AudioProcessor::releaseResources()
{
    dsp.reset();
}

bool Ember175AudioProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    const auto in = layout.getMainInputChannelSet();
    const auto out = layout.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void Ember175AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    ember::Parameters p;
    const bool continuous = read ("continuous") > 0.5f;
    auto taper = [continuous] (float db)
    {
        // the hardware attenuators are stepped in 2 dB detents; the plug-in can
        // run them continuously for fine automation
        return continuous ? db : std::round (db * 0.5f) * 2.0f;
    };

    p.inputDb      = taper (read ("input"));
    p.inputTrimDb  = read ("vernier");
    p.outputDb     = taper (read ("output"));
    p.outputTrimDb = 0.0f;
    p.detectorHighPass = read ("lowHigh") > 0.5f;
    p.lowZInput    = read ("impedance") > 0.5f;
    p.grTrimDb     = read ("grTrim");
    p.attackMs     = ember::model::attackNormToMs (read ("attack"));
    p.releaseMs    = ember::model::releaseFromNorm (read ("release"));
    p.headroomDb   = read ("headroom");
    p.mix          = read ("mix") * 0.01f;
    p.stereoLink   = read ("stereoLink") > 0.5f;
    p.bypass       = read ("power") < 0.5f;

    float* ptrs[2] { nullptr, nullptr };
    const int num = juce::jmin (2, numOut);
    for (int ch = 0; ch < num; ++ch)
        ptrs[ch] = buffer.getWritePointer (ch);

    dsp.process (ptrs, num, buffer.getNumSamples(), p);

    grMeter.store (dsp.gainReductionDb());
    outMeter.store (dsp.outputLevelDb());
}

juce::AudioProcessorEditor* Ember175AudioProcessor::createEditor()
{
    return new Ember175AudioProcessorEditor (*this);
}

void Ember175AudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    state.setProperty ("version", 1, nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}

void Ember175AudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ember175AudioProcessor();
}
