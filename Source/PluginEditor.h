#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/PanelMapping.h"
#include "UI/PanelRenderer.h"

//==============================================================================
/** A rotary control that paints itself as one of the front panel knobs.

    The printed scale, caption and the knob body are all drawn by the shared
    renderer, so the editor and the headless preview cannot drift apart. */
class PanelKnob : public juce::Slider
{
public:
    PanelKnob (ember::ui::KnobSpec spec, const juce::String& paramId,
               double minValue, double maxValue, double scaleForDisplay = 1.0);

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
    double displayOffset = 0.0;
    bool hovered = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelKnob)
};

//==============================================================================
/** Hex bat-handle toggle used for S.C. LINK and POWER. */
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
    void refreshTooltip();

    Ember175AudioProcessor& processor;
    ember::ui::PanelLayout layout;
    ember::ui::PanelMapping mapping;

    std::vector<std::unique_ptr<PanelKnob>> knobs;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> knobAttachments;

    PanelToggle linkButton { "S.C. LINK", "stereoLink" };
    PanelToggle powerButton { "POWER", "power" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> linkAtt, powerAtt;

    PanelKnob* hoveredKnob = nullptr;
    juce::String hoverText;
    juce::TooltipWindow tooltip { this, 700 };

    float needleGr = 0.0f, needleLevel = -100.0f;
    float activity = 0.0f;
    int repaintDivider = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ember175AudioProcessorEditor)
};
