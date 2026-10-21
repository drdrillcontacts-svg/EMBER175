#include "PluginProcessor.h"
#include "PluginEditor.h"

Ember175AudioProcessor::Ember175AudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

Ember175AudioProcessor::APVTS::ParameterLayout Ember175AudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout l;
    l.add(std::make_unique<juce::AudioParameterFloat>("input", "Input", juce::NormalisableRange<float>(-24, 24, 0.1f), 0.0f, "dB"));
    l.add(std::make_unique<juce::AudioParameterFloat>("output", "Output", juce::NormalisableRange<float>(-24, 24, 0.1f), 0.0f, "dB"));
    l.add(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", juce::NormalisableRange<float>(-48, 0, 0.1f), -18.0f, "dB"));
    l.add(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.45f), 2.0f, "ms"));
    l.add(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(5.0f, 2000.0f, 1.0f, 0.45f), 120.0f, "ms"));
    l.add(std::make_unique<juce::AudioParameterFloat>("headroom", "Headroom", juce::NormalisableRange<float>(-12, 12, 0.1f), 0.0f, "dB"));
    l.add(std::make_unique<juce::AudioParameterFloat>("mix", "Dry / Wet", juce::NormalisableRange<float>(0, 100, 0.1f), 100.0f, "%"));
    l.add(std::make_unique<juce::AudioParameterFloat>("color", "Color", juce::NormalisableRange<float>(0, 100, 0.1f), 15.0f, "%"));
    l.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    l.add(std::make_unique<juce::AudioParameterBool>("stereoLink", "Stereo Link", true));
    return l;
}

void Ember175AudioProcessor::prepareToPlay(double sr, int block)
{
    dsp.prepare(sr, block, getTotalNumOutputChannels());
}
void Ember175AudioProcessor::releaseResources() { dsp.reset(); }

bool Ember175AudioProcessor::isBusesLayoutSupported(const BusesLayout& b) const
{
    const auto in = b.getMainInputChannelSet();
    const auto out = b.getMainOutputChannelSet();
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void Ember175AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int ins = getTotalNumInputChannels();
    const int outs = getTotalNumOutputChannels();
    for (int ch = ins; ch < outs; ++ch) buffer.clear(ch, 0, buffer.getNumSamples());

    ember::Parameters p;
    p.inputDb = parameters.getRawParameterValue("input")->load();
    p.outputDb = parameters.getRawParameterValue("output")->load();
    p.thresholdDb = parameters.getRawParameterValue("threshold")->load();
    p.attackMs = parameters.getRawParameterValue("attack")->load();
    p.releaseMs = parameters.getRawParameterValue("release")->load();
    p.headroomDb = parameters.getRawParameterValue("headroom")->load();
    p.mix = parameters.getRawParameterValue("mix")->load() / 100.0f;
    p.color = parameters.getRawParameterValue("color")->load() / 100.0f;
    p.bypass = parameters.getRawParameterValue("bypass")->load() > 0.5f;
    p.stereoLink = parameters.getRawParameterValue("stereoLink")->load() > 0.5f;

    float* ptrs[2] { buffer.getWritePointer(0), nullptr };
    if (outs > 1) ptrs[1] = buffer.getWritePointer(1);
    dsp.process(ptrs, outs, buffer.getNumSamples(), p);
}

juce::AudioProcessorEditor* Ember175AudioProcessor::createEditor() { return new Ember175AudioProcessorEditor(*this); }

void Ember175AudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    std::unique_ptr<juce::XmlElement> xml(parameters.copyState().createXml());
    copyXmlToBinary(*xml, dest);
}
void Ember175AudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Ember175AudioProcessor(); }
