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
        float levelDb = -60.0f;     // VU scale, 0 VU = 0 dBFS
        bool grMode = true;
        bool powered = true;
        bool limiting = false;
    };

    class PanelRenderer
    {
    public:
        static void renderBackground (juce::Graphics&, const PanelLayout&, float scale);

        static void drawKnob (juce::Graphics&, const KnobSpec&, float normalised,
                              bool highlighted = false, bool dragging = false);

        static void drawKnobRing (juce::Graphics&, const KnobSpec&, float normalised);

        static void drawMeterNeedle (juce::Graphics&, const PanelLayout&, const MeterState&);
        static void drawMeterGlass (juce::Graphics&, const PanelLayout&, const MeterState&);

        static void drawToggle (juce::Graphics&, juce::Rectangle<float> area, bool on,
                                const juce::String& label, bool powerStyle = false);

        static void drawReadout (juce::Graphics&, juce::Rectangle<float> area,
                                 const juce::String& big, const juce::String& small,
                                 float activity);

        static juce::Font panelFont (float height, bool bold = true, float kerning = 0.14f);

        // palette
        static juce::Colour faceBase();
        static juce::Colour faceDark();
        static juce::Colour print();
        static juce::Colour printMuted();
        static juce::Colour brass();
        static juce::Colour bakelite();
    };
}
