#pragma once

/*
    Geometry for the EMBER 175 front panel, in design units.

    Everything is described in a 1200 x 360 design space so that the renderer,
    the editor and the offline preview tool all agree. The editor maps that
    space onto the window with a uniform scale, so the panel keeps its
    proportions at any window size.

    Layout, left to right:
        LOW/HI, S.C. LINK, lamp | INPUT + IMPEDANCE + VERNIER | VU meter and
        brand block | ATTACK, OUTPUT, MIX, RELEASE, POWER.
*/

#include <juce_graphics/juce_graphics.h>

namespace ember::ui
{
    constexpr float kDesignWidth = 1200.0f;
    constexpr float kDesignHeight = 360.0f;

    enum class KnobStyle
    {
        big,        // INPUT / OUTPUT: large knurled aluminium
        medium,     // attack, release, mix, verniers
        small,      // two position switches (LOW/HI, IMPEDANCE)
        tiny
    };

    struct KnobSpec
    {
        juce::String label;
        KnobStyle style = KnobStyle::medium;
        juce::Point<float> centre;
        float radius = 18.0f;
        float ringRadius = 0.0f;         // printed scale radius
        float angleMin = 0.0f;           // radians, 0 = 12 o'clock
        float angleMax = 0.0f;
        int tickCount = 0;               // printed ticks over the travel
        juce::StringArray tickLabels;    // evenly spaced around the travel
        juce::String leftLabel, rightLabel;   // printed either side of the knob
        juce::String offLabel;                // extra legend, e.g. "OFF"
        bool captionAbove = true;             // control name above or below
    };

    struct PanelLayout
    {
        juce::Rectangle<float> panel;        // the metal enclosure
        juce::Rectangle<float> face;         // painted front face
        juce::Rectangle<float> meter;        // bezel
        juce::Rectangle<float> meterFace;    // printed scale card

        juce::Point<float> lamp;             // amber activity jewel
        float lampRadius = 8.0f;

        juce::Rectangle<float> linkToggle;   // S.C. LINK bat switch
        juce::Point<float> linkOffLabel, linkOnLabel;
        juce::Rectangle<float> powerToggle;  // POWER bat switch
        juce::Point<float> powerOnLabel;
        juce::Point<float> powerLamp;
        float powerLampRadius = 7.0f;

        juce::Point<float> brandLogo;        // diamond monogram centre
        float brandLogoRadius = 20.0f;
        juce::Rectangle<float> brandName;
        juce::Rectangle<float> brandSub;

        juce::Rectangle<float> statusLine;   // tiny readout, bottom right

        KnobSpec lowHigh, input, impedance, vernier, attack, output, mix, release;

        KnobSpec* knobFor (int index) noexcept;
        static constexpr int numKnobs = 8;
    };

    PanelLayout makeLayout();
}
