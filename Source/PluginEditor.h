#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class Ember175AudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit Ember175AudioProcessorEditor(Ember175AudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    Ember175AudioProcessor& processor;
    juce::Label title, subtitle, meterLabel, footerLabel;
    juce::Slider input, output, threshold, attack, release, headroom, mix, color;
    juce::ToggleButton bypass, stereoLink;
    std::vector<std::unique_ptr<juce::Label>> labels;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<Attachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment, linkAttachment;
    void setupSlider(juce::Slider&, const juce::String&, const juce::String&);
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Ember175AudioProcessorEditor)
};
