#pragma once

/*
    All of the front panel painting lives here. It only depends on
    juce_graphics, which keeps it testable outside the plug-in (see
    Tools/UIPreview.cpp).
*/

#include "PanelLayout.h"

namespace ember::ui
{
    struct MeterState
    {
        float grDb = 0.0f;          // 0 .. 20
        float levelDb = -60.0f;     // 0 VU = 0 dBFS
        bool grMode = false;
        bool powered = true;
        bool limiting = false;
    };

    class PanelRenderer
    {
    public:
        static void renderBackground (juce::Graphics&, const PanelLayout&, float scale);

        /** Draws a knob at its design-space position. */
        static void drawKnob (juce::Graphics&, const KnobSpec&, float normalised,
                              bool highlighted = false, bool dragging = false);

        /** Draws a knob into a component rectangle that is centred on the knob and
            sized to cover it plus a margin (this is what PanelKnob uses, and what
            the headless preview uses so it exercises the editor path). */
        static void paintKnobComponent (juce::Graphics&, const KnobSpec&, juce::Rectangle<float> localArea,
                                        float normalised, bool highlighted = false, bool dragging = false);

        static void drawToggle (juce::Graphics&, juce::Rectangle<float> area, bool on,
                                const juce::String& label, bool showLabel = true);

        static void drawLamp (juce::Graphics&, juce::Point<float> centre, float radius,
                              juce::Colour colour, float glow);

        static void drawMeterNeedle (juce::Graphics&, const PanelLayout&, const MeterState&);
        static void drawMeterGlass (juce::Graphics&, const PanelLayout&, const MeterState&);

        static void drawStatus (juce::Graphics&, juce::Rectangle<float> area, const juce::String& text);

        static juce::Font panelFont (float height, bool bold = true, float kerning = 0.2f);

        // palette
        static juce::Colour faceDark();
        static juce::Colour print();
        static juce::Colour printMuted();
        static juce::Colour brass();
    };
}
