#include "PluginEditor.h"

using namespace ember::ui;

//==============================================================================
PanelKnob::PanelKnob (KnobSpec s, const juce::String& id, double minValue, double maxValue)
    : spec (s), paramId (id)
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters (0.0f, juce::MathConstants<float>::twoPi, false);
    // the two-position impedance / response switches detent, everything else is
    // continuous (the 2 dB attenuator steps are applied in the processor)
    setRange (minValue, maxValue, spec.style == KnobStyle::small ? 1.0 : 0.001);
    setVelocityBasedMode (false);
    setMouseDragSensitivity (spec.style == KnobStyle::big ? 280 : 200);
    setWantsKeyboardFocus (false);
    setOpaque (false);
    setPopupMenuEnabled (false);
}

void PanelKnob::paint (juce::Graphics& g)
{
    PanelRenderer::paintKnobComponent (g, spec, getLocalBounds().toFloat(),
                                       getNormalised(), hovered, isMouseButtonDown());
}

float PanelKnob::getNormalised() const noexcept
{
    const double span = getMaximum() - getMinimum();
    if (span <= 0.0)
        return 0.0f;
    return juce::jlimit (0.0f, 1.0f, static_cast<float> ((getValue() - getMinimum()) / span));
}

void PanelKnob::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        if (onPopupMenu != nullptr)
            onPopupMenu (e.getScreenPosition());
        return;
    }

    juce::Slider::mouseDown (e);
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

//==============================================================================
PanelToggle::PanelToggle (const juce::String& text, const juce::String& parameterId)
    : juce::Button (text), paramId (parameterId)
{
    setClickingTogglesState (true);
    setWantsKeyboardFocus (false);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip (text);
}

void PanelToggle::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    if (highlighted || down)
    {
        g.setColour (juce::Colour (0x18ffd9a0));
        g.fillEllipse (getLocalBounds().toFloat().withSizeKeepingCentre (getWidth() * 0.9f, getHeight() * 0.5f));
    }

    PanelRenderer::drawToggle (g, getLocalBounds().toFloat(), getToggleState(), getButtonText());
}

void PanelToggle::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        if (onPopupMenu != nullptr)
            onPopupMenu (e.getScreenPosition());
        return;
    }

    juce::Button::mouseDown (e);
}

//==============================================================================
Ember175AudioProcessorEditor::Ember175AudioProcessorEditor (Ember175AudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), layout (makeLayout())
{
    setOpaque (true);
    setResizable (true, true);
    setResizeLimits (720, 216, 1920, 576);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio (kDesignWidth / kDesignHeight);
    setSize (1200, 360);

    // layout index -> (parameter, minimum, maximum)
    attachKnob (0, "lowHigh",    0.0, 1.0);
    attachKnob (1, "input",      ember::model::kInputMinDb,  ember::model::kInputMaxDb);
    attachKnob (2, "impedance",  0.0, 1.0);
    attachKnob (3, "vernier",    -ember::model::kVernierDb, ember::model::kVernierDb);
    attachKnob (4, "attack",     0.0, 1.0);
    attachKnob (5, "output",     ember::model::kOutputMinDb, ember::model::kOutputMaxDb);
    attachKnob (6, "mix",        0.0, 100.0);
    attachKnob (7, "release",    0.0, 1.0);

    for (auto* b : { &linkButton, &powerButton })
    {
        addAndMakeVisible (*b);
        b->onStateChange = [this] { repaint(); };
        b->onPopupMenu = [this] (juce::Point<int> p) { showSetupMenu (p); };
    }

    linkAtt  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "stereoLink", linkButton);
    powerAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.parameters, "power", powerButton);

    startTimerHz (30);
}

Ember175AudioProcessorEditor::~Ember175AudioProcessorEditor()
{
    stopTimer();
    for (auto& k : knobs)
        k->onHover = nullptr;
}

void Ember175AudioProcessorEditor::attachKnob (int layoutIndex, const char* paramId, double min, double max)
{
    auto* spec = layout.knobFor (layoutIndex);
    if (spec == nullptr)
        return;

    auto knob = std::make_unique<PanelKnob> (*spec, paramId, min, max);
    knob->onHover = [this] (PanelKnob* k) { hoveredKnob = k; refreshStatus(); repaint(); };
    knob->onPopupMenu = [this] (juce::Point<int> p) { showSetupMenu (p); };
    addAndMakeVisible (*knob);

    knobAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.parameters, paramId, *knob));

    if (auto* param = processor.parameters.getParameter (paramId))
        knob->setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));

    knobs.push_back (std::move (knob));
}

void Ember175AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f0c0c));

    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::translation (mapping.origin.x, mapping.origin.y));
    PanelRenderer::renderBackground (g, layout, mapping.scale);

    // meter
    MeterState m;
    m.grDb = needleGr;
    m.levelDb = needleLevel;
    m.grMode = processor.read ("grMeterMode") > 0.5f;
    m.powered = processor.read ("power") > 0.5f;
    m.limiting = activity > 0.05f;
    PanelRenderer::drawMeterNeedle (g, layout, m);
    PanelRenderer::drawMeterGlass (g, layout, m);

    // lamps: amber shows gain reduction activity, red is mains
    PanelRenderer::drawLamp (g, layout.lamp, layout.lampRadius, juce::Colour (0xffffa53a), activity);
    PanelRenderer::drawLamp (g, layout.powerLamp, layout.powerLampRadius, juce::Colour (0xffff3a24),
                             m.powered ? 1.0f : 0.0f);

    PanelRenderer::drawStatus (g, layout.statusLine, statusText);
}

void Ember175AudioProcessorEditor::resized()
{
    mapping = PanelMapping::fit (getWidth(), getHeight());

    const int count = juce::jmin (static_cast<int> (knobs.size()), PanelLayout::numKnobs);
    for (int i = 0; i < count; ++i)
    {
        auto& k = *knobs[static_cast<size_t> (i)];
        const auto& spec = k.getSpec();
        k.setBounds (place ({ spec.centre.x - spec.radius * 1.3f,
                              spec.centre.y - spec.radius * 1.3f,
                              spec.radius * 2.6f, spec.radius * 2.6f }));
    }

    linkButton.setBounds (place (layout.linkToggle));
    powerButton.setBounds (place (layout.powerToggle));
}

juce::Rectangle<int> Ember175AudioProcessorEditor::place (juce::Rectangle<float> designArea) const
{
    return mapping.area (designArea);
}

void Ember175AudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
        showSetupMenu (juce::Desktop::getMousePosition());
}

void Ember175AudioProcessorEditor::showSetupMenu (juce::Point<int> screenPosition)
{
    auto* state = &processor.parameters;

    juce::PopupMenu menu;
    menu.addSectionHeader ("EMBER 175-B setup");

    juce::PopupMenu meterMenu;
    const bool grMode = processor.read ("grMeterMode") > 0.5f;
    meterMenu.addItem (1, "VU (output level)", true, ! grMode);
    meterMenu.addItem (2, "Gain reduction", true, grMode);
    menu.addSubMenu ("Meter shows", meterMenu);

    juce::PopupMenu taperMenu;
    const bool continuous = processor.read ("continuous") > 0.5f;
    taperMenu.addItem (3, "Stepped (2 dB detents)", true, ! continuous);
    taperMenu.addItem (4, "Continuous", true, continuous);
    menu.addSubMenu ("Input / output taper", taperMenu);

    juce::PopupMenu headroomMenu;
    const float headroom = processor.read ("headroom");
    for (int db = -12; db <= 12; db += 2)
        headroomMenu.addItem (100 + db, (db > 0 ? "+" : "") + juce::String (db) + " dB", true,
                              std::abs (headroom - static_cast<float> (db)) < 0.25f);
    menu.addSubMenu ("Headroom", headroomMenu);

    juce::PopupMenu grTrimMenu;
    const float grTrim = processor.read ("grTrim");
    for (int db = -6; db <= 6; db += 1)
        grTrimMenu.addItem (200 + db, (db > 0 ? "+" : "") + juce::String (db) + " dB", true,
                            std::abs (grTrim - static_cast<float> (db)) < 0.25f);
    menu.addSubMenu ("G/R vernier (threshold trim)", grTrimMenu);

    menu.addSeparator();
    menu.addItem (5, "Stereo link", true, processor.read ("stereoLink") > 0.5f);
    menu.addItem (6, "Power", true, processor.read ("power") > 0.5f);

    auto setFloat = [this] (const char* id, float value)
    {
        if (auto* p = processor.parameters.getParameter (id))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (value));
            p->endChangeGesture();
        }
    };

    auto setBool = [this] (const char* id, bool value)
    {
        if (auto* p = processor.parameters.getParameter (id))
            p->setValueNotifyingHost (value ? 1.0f : 0.0f);
    };

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea ({ screenPosition.x, screenPosition.y, 1, 1 }),
                        [this, setFloat, setBool] (int result)
    {
        if (result == 0)
            return;

        switch (result)
        {
            case 1: setBool ("grMeterMode", false); break;
            case 2: setBool ("grMeterMode", true);  break;
            case 3: setBool ("continuous", false);  break;
            case 4: setBool ("continuous", true);   break;
            case 5: setBool ("stereoLink", processor.read ("stereoLink") < 0.5f); break;
            case 6: setBool ("power", processor.read ("power") < 0.5f); break;
            default: break;
        }

        if (result >= 100 && result < 200)
            setFloat ("headroom", static_cast<float> (result - 100));
        if (result >= 200 && result < 300)
            setFloat ("grTrim", static_cast<float> (result - 200));

        repaint();
    });
}

void Ember175AudioProcessorEditor::refreshStatus()
{
    if (hoveredKnob != nullptr)
    {
        const auto& spec = hoveredKnob->getSpec();
        juce::String value;
        if (spec.style == KnobStyle::small)
            value = hoveredKnob->getNormalised() > 0.5f ? spec.rightLabel : spec.leftLabel;
        else if (auto* param = processor.parameters.getParameter (hoveredKnob->getParamId()))
            value = param->getCurrentValueAsText();

        const juce::String name = spec.label.isNotEmpty() ? spec.label : spec.captionAbove ? juce::String() : juce::String();
        statusText = (name.isNotEmpty() ? name + "   " : juce::String()) + value;
        return;
    }

    const float gr = processor.getGainReductionDb();
    const float lvl = processor.getOutputLevelDb();
    const float attackMs = ember::model::attackNormToMs (processor.read ("attack"));

    statusText = "GR " + juce::String (gr < 0.05f ? 0.0f : gr, 1) + " dB"
               + "      OUT " + (lvl < -90.0f ? juce::String ("-inf") : juce::String (lvl, 1)) + " dBFS"
               + "      ATTACK " + (attackMs <= 0.0f ? juce::String ("OFF")
                                                    : juce::String (attackMs, 2) + " ms")
               + "      RELEASE " + juce::String (ember::model::releaseFromNorm (processor.read ("release")), 0) + " ms";
}

void Ember175AudioProcessorEditor::timerCallback()
{
    const float gr = processor.getGainReductionDb();
    const float level = processor.getOutputLevelDb();

    needleGr += (gr - needleGr) * 0.45f;
    needleLevel += (level - needleLevel) * 0.30f;

    // lamp flickers with the amount of gain reduction
    const float target = juce::jlimit (0.0f, 1.0f, gr / 8.0f);
    activity += (target - activity) * 0.35f;

    refreshStatus();

    if (++repaintDivider >= 2)
    {
        repaintDivider = 0;
        repaint (place (layout.meter.expanded (4.0f)));
        repaint (place ({ layout.lamp.x - 26.0f, layout.lamp.y - 26.0f, 52.0f, 52.0f }));
        repaint (place (layout.statusLine.expanded (2.0f)));
    }
}
