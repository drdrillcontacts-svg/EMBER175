#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/PanelRenderer.h"

//==============================================================================
/** A rotary control that paints itself as one of the panel knobs. */
class PanelKnob final : public juce::Slider
{
public:
    PanelKnob (ember::ui::KnobSpec spec, const juce::String& paramId, double minValue, double maxValue);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    const juce::String& getParamId() const noexcept { return paramId; }
    const ember::ui::KnobSpec& getSpec() const noexcept { return spec; }
    float getNormalised() const noexcept;

    std::function<void (PanelKnob*)> onHover;
    std::function<void (juce::Point<int>)> onPopupMenu;

private:
    ember::ui::KnobSpec spec;
    juce::String paramId;
    bool hovered = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelKnob)
};

//==============================================================================
/** Bat-handle toggle used for S.C. LINK and POWER. */
class PanelToggle final : public juce::Button
{
public:
    PanelToggle (const juce::String& text, const juce::String& parameterId);

    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void mouseDown (const juce::MouseEvent&) override;

    const juce::String& getParamId() const noexcept { return paramId; }
    std::function<void (juce::Point<int>)> onPopupMenu;

private:
    juce::String paramId;
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
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void showSetupMenu (juce::Point<int> screenPosition);
    juce::Rectangle<int> place (juce::Rectangle<float> designArea) const;
    void attachKnob (int layoutIndex, const char* paramId, double min, double max);
    void refreshStatus();

    Ember175AudioProcessor& processor;
    ember::ui::PanelLayout layout;
    float currentScale = 1.0f;
    juce::Point<float> origin;

    std::vector<std::unique_ptr<PanelKnob>> knobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> knobAttachments;

    PanelToggle linkButton { "S.C. LINK", "stereoLink" };
    PanelToggle powerButton { "POWER", "power" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> linkAtt, powerAtt;

    juce::String statusText { "GR 0.0 dB     OUT -inf dBFS" };
    PanelKnob* hoveredKnob = nullptr;

    float needleGr = 0.0f, needleLevel = -100.0f;
    float activity = 0.0f;
    int repaintDivider = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ember175AudioProcessorEditor)
};
