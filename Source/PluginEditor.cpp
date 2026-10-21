#include "PluginEditor.h"

using namespace ember::ui;

//==============================================================================
PanelKnob::PanelKnob (KnobSpec s, const juce::String& id)
    : spec (s), paramId (id)
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters (0.0f, juce::MathConstants<float>::twoPi, false);
    setVelocityBasedMode (false);
    setMouseDragSensitivity (spec.style == KnobStyle::large ? 260 : 190);
    setWantsKeyboardFocus (false);
    setOpaque (false);
}

void PanelKnob::paint (juce::Graphics& g)
{
    const float norm = static_cast<float> ((getValue() - getMinimum()) / (getMaximum() - getMinimum()));
    PanelRenderer::paintKnobComponent (g, spec, getLocalBounds().toFloat(), norm, hovered, isMouseButtonDown());
}

void PanelKnob::mouseEnter (const juce::MouseEvent& e)
{
    hovered = true;
    repaint();
    if (onHover != nullptr)
        onHover (this);
    juce::Slider::mouseEnter (e);
}

void PanelKnob::mouseExit (const juce::MouseEvent& e)
{
    hovered = false;
    repaint();
    if (onHover != nullptr)
        onHover (nullptr);
    juce::Slider::mouseExit (e);
}

juce::String PanelKnob::getDisplayValue()
{
    return getTextFromValue (getValue());
}

//==============================================================================
PanelToggle::PanelToggle (const juce::String& text, Style s)
    : juce::Button (text), style (s)
{
    setClickingTogglesState (true);
    setWantsKeyboardFocus (false);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void PanelToggle::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto area = getLocalBounds().toFloat();
    const bool on = getToggleState();

    if (style == Style::power)
    {
        PanelRenderer::drawToggle (g, area, on, getButtonText(), true);
        return;
    }

    if (highlighted || down)
    {
        g.setColour (PanelRenderer::brass().withAlpha (down ? 0.20f : 0.10f));
        g.fillRoundedRectangle (area, 5.0f);
    }

    PanelRenderer::drawToggle (g, area, on, getButtonText(), false);
}

juce::Rectangle<float> PanelToggle::getIndicatorArea() const
{
    return getLocalBounds().toFloat();
}

//==============================================================================
Ember175AudioProcessorEditor::Ember175AudioProcessorEditor (Ember175AudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), layout (makeLayout())
{
    setOpaque (true);
    setResizable (true, true);
    setResizeLimits (900, 450, 1920, 960);
    getConstrainer()->setFixedAspectRatio (kDesignWidth / kDesignHeight);
    setSize (1200, 600);

    struct Binding { int index; const char* id; };
    const Binding bindings[] = {
        { 0, "input" }, { 1, "output" }, { 2, "attack" }, { 3, "release" },
        { 4, "inputTrim" }, { 5, "outputTrim" }, { 6, "grTrim" },
        { 7, "mix" }, { 8, "headroom" }
    };

    for (auto& b : bindings)
    {
        if (auto* spec = layout.knobFor (b.index))
        {
            auto knob = std::make_unique<PanelKnob> (*spec, b.id);
            knob->onHover = [this] (PanelKnob* k) { setHovered (k); };
            addAndMakeVisible (*knob);
            knobAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                processor.parameters, b.id, *knob));
            knobs.push_back (std::move (knob));
        }
    }

    for (auto* b : { &linkButton, &continuousButton, &meterButton, &powerButton })
    {
        b->setColour (juce::Label::textColourId, PanelRenderer::print());
        addAndMakeVisible (*b);
    }

    linkAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "stereoLink", linkButton);
    contAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "continuous", continuousButton);
    meterAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "grMeterMode", meterButton);
    powerAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "power", powerButton);

    // double click a knob to return to its default, and refresh the readout while
    // dragging
    for (auto& k : knobs)
    {
        if (auto* param = processor.parameters.getParameter (k->getParamId()))
            k->setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));

        k->onValueChange = [this, raw = k.get()] { setHovered (raw); };
    }

    powerButton.onStateChange = [this] { repaint(); };
    linkButton.onStateChange = [this] { repaint(); };
    meterButton.onStateChange = [this]
    {
        needleGr = needleLevel = -100.0f;
        repaint();
    };

    startTimerHz (30);
}

Ember175AudioProcessorEditor::~Ember175AudioProcessorEditor()
{
    stopTimer();
    for (auto& k : knobs)
        k->onValueChange = nullptr;
}

void Ember175AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff151312));

    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::translation (origin.x, origin.y));
    PanelRenderer::renderBackground (g, layout, currentScale);

    // meter: needle is animated by the timer, the glass is static
    MeterState m;
    m.grDb = needleGr;
    m.levelDb = needleLevel;
    m.grMode = processor.read ("grMeterMode") > 0.5f;
    m.powered = processor.read ("power") > 0.5f;
    m.limiting = m.grDb > 0.15f;
    PanelRenderer::drawMeterNeedle (g, layout, m);
    PanelRenderer::drawMeterGlass (g, layout, m);

    PanelRenderer::drawReadout (g, layout.readout, readoutBig, readoutSmall, m.limiting ? 1.0f : 0.25f);
}

void Ember175AudioProcessorEditor::resized()
{
    const float s = juce::jmin (static_cast<float> (getWidth()) / kDesignWidth,
                                static_cast<float> (getHeight()) / kDesignHeight);
    currentScale = s;
    origin = { (getWidth() - kDesignWidth * s) * 0.5f, (getHeight() - kDesignHeight * s) * 0.5f };

    for (auto& k : knobs)
    {
        const auto& spec = k->getSpec();
        // the knob component covers the bakelite plus a little breathing room
        auto area = juce::Rectangle<float> (spec.centre.x - spec.radius * 1.3f,
                                            spec.centre.y - spec.radius * 1.3f,
                                            spec.radius * 2.6f, spec.radius * 2.6f);
        k->setBounds (place (area));
    }

    powerButton.setBounds (place (layout.power));
    linkButton.setBounds (place (layout.linkButton));
    continuousButton.setBounds (place (layout.vernierButton));
    meterButton.setBounds (place (layout.meterButton));
}

void Ember175AudioProcessorEditor::timerCallback()
{
    meterGr = processor.getGainReductionDb();
    meterLevel = processor.getOutputLevelDb();

    // mechanical ballistics for the needle
    const bool grMode = processor.read ("grMeterMode") > 0.5f;
    needleGr += (meterGr - needleGr) * 0.42f;
    needleLevel += (meterLevel - needleLevel) * (grMode ? 0.5f : 0.3f);

    // readout: hovered control wins, otherwise the two quantities that matter
    if (hoveredKnob != nullptr)
    {
        juce::String name = hoveredKnob->getSpec().label;
        readoutBig = name + "  " + hoveredKnob->getDisplayValue();
        readoutSmall = "INPUT " + juce::String (processor.read ("input"), 1)
                     + " dB    OUTPUT " + juce::String (processor.read ("output"), 1) + " dB";
    }
    else
    {
        const float g = meterGr < 0.05f ? 0.0f : meterGr;
        readoutBig = "GR  " + juce::String (g, 1) + " dB";
        juce::String lvl = meterLevel < -90.0f ? juce::String ("-inf")
                                               : juce::String (meterLevel, 1);
        const float attackMs = ember::model::attackNormToMs (processor.read ("attack"));
        const juce::String attackText = attackMs <= 0.0f ? juce::String ("OFF")
                                                         : juce::String (attackMs, 2) + " ms";
        readoutSmall = "OUT " + lvl + " dBFS    A " + attackText
                     + "    R " + juce::String (ember::model::releaseFromNorm (processor.read ("release")), 0) + " ms";
    }

    if (++repaintDivider >= 2)
    {
        repaintDivider = 0;
        repaint (place (layout.meter.expanded (6.0f)));
        repaint (place (layout.readout));
    }
}

void Ember175AudioProcessorEditor::setHovered (PanelKnob* k)
{
    hoveredKnob = k;
    repaint (place (layout.readout));
}

juce::Rectangle<int> Ember175AudioProcessorEditor::place (juce::Rectangle<float> designArea) const
{
    return juce::Rectangle<int> (juce::roundToInt (origin.x + designArea.getX() * currentScale),
                                 juce::roundToInt (origin.y + designArea.getY() * currentScale),
                                 juce::roundToInt (designArea.getWidth() * currentScale),
                                 juce::roundToInt (designArea.getHeight() * currentScale));
}
