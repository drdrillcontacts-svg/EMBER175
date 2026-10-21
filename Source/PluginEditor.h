#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/PanelRenderer.h"

//==============================================================================
/** A rotary control that paints itself as the 175-B hardware knob. */
class PanelKnob final : public juce::Slider
{
public:
    PanelKnob (ember::ui::KnobSpec spec, const juce::String& paramId);

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    const juce::String& getParamId() const noexcept { return paramId; }
    juce::String getDisplayValue();
    const ember::ui::KnobSpec& getSpec() const noexcept { return spec; }

    std::function<void (PanelKnob*)> onHover;

private:
    ember::ui::KnobSpec spec;
    juce::String paramId;
    bool hovered = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelKnob)
};

//==============================================================================
/** LED style toggle used for the extended control switches. */
class PanelToggle final : public juce::Button
{
public:
    enum class Style { led, power };

    PanelToggle (const juce::String& text, Style s);

    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    juce::Rectangle<float> getIndicatorArea() const;

private:
    Style style;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelToggle)
};

//==============================================================================
class Ember175AudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit Ember175AudioProcessorEditor (Ember175AudioProcessor&);
    ~Ember175AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setHovered (PanelKnob*);
    juce::Rectangle<int> place (juce::Rectangle<float> designArea) const;

    Ember175AudioProcessor& processor;
    ember::ui::PanelLayout layout;
    float currentScale = 1.0f;
    juce::Point<float> origin;

    std::vector<std::unique_ptr<PanelKnob>> knobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> knobAttachments;

    PanelToggle linkButton { "LINKED", PanelToggle::Style::led };
    PanelToggle continuousButton { "CONTINUOUS", PanelToggle::Style::led };
    PanelToggle meterButton { "GAIN REDUCTION", PanelToggle::Style::led };
    PanelToggle powerButton { "ON", PanelToggle::Style::power };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> linkAtt, contAtt, meterAtt, powerAtt;

    juce::String readoutBig { "INPUT  20.0 dB" }, readoutSmall { "GR 0.0 dB    OUT -inf dBFS" };
    PanelKnob* hoveredKnob = nullptr;
    float meterGr = 0.0f, meterLevel = -100.0f;
    float needleGr = 0.0f, needleLevel = -100.0f;
    int repaintDivider = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ember175AudioProcessorEditor)
};
