#pragma once

/*
    Geometry for the EMBER 175-B front panel, in design units.

    Everything is described in a 1300 x 260 design space (the reference chassis
    is 5:1 with its rack ears) so that the renderer, the editor and the offline
    preview tool all agree. The editor maps that space onto the window with a uniform scale, so
    the panel keeps its proportions at any window size.

    Layout, measured from the reference panel, left to right:

        LOW/HI            small two-position dial
        S.C. LINK         hex bat switch, OFF above, ON below, amber jewel
        INPUT             large dial, -20 .. +20 dB, 4 dB print
        IMPEDANCE         small dial, 600 / 300
        VERNIER           small dial, -5 .. +5
        VU meter          large amber card in a bronze bezel
        D-A monogram      diamond, EMBER 175, TUBE COMPRESSOR
        ATTACK            small dial with an OFF detent
        OUTPUT            large dial, -20 .. +20 dB
        MIX               small dial, 0 .. 100
        RELEASE           small dial with an OFF detent
        POWER             red jewel and hex bat switch
*/

#include <juce_graphics/juce_graphics.h>
#include <vector>

namespace ember::ui
{
    constexpr float kDesignWidth  = 1300.0f;
    constexpr float kDesignHeight = 260.0f;
    constexpr float kEarWidth     = 50.0f;      // rack ear either side

    enum class KnobStyle
    {
        big,        // INPUT / OUTPUT: big black skirt, wide brushed cap
        small       // everything else
    };

    /** One printed number on a dial scale, positioned along the travel. */
    struct ScaleMark
    {
        float position = 0.0f;      // 0 = fully anticlockwise, 1 = fully clockwise
        juce::String text;
    };

    struct KnobSpec
    {
        juce::String caption;                    // printed name, e.g. "INPUT"
        juce::Point<float> captionPos;

        KnobStyle style = KnobStyle::small;
        juce::Point<float> centre;
        float skirtRadius = 26.0f;               // black skirt
        float capRadius = 19.0f;                 // brushed aluminium cap
        float tickRadius = 36.0f;                // printed tick ring
        float labelRadius = 47.0f;               // printed numbers
        float labelHeight = 12.0f;

        float angleMin = 0.0f;                   // radians, 0 = 12 o'clock
        float angleMax = 0.0f;
        int tickCount = 13;
        std::vector<ScaleMark> marks;

        juce::String leftLabel, rightLabel;      // printed either side of the dial
        juce::Point<float> leftLabelPos, rightLabelPos;
        juce::String offLabel;                   // OFF detent legend
        juce::Point<float> offLabelPos;
    };

    struct PanelLayout
    {
        juce::Rectangle<float> panel;         // everything, ears included
        juce::Rectangle<float> ears[2];       // brushed aluminium rack ears
        juce::Rectangle<float> panelCase;     // the wine case itself
        juce::Rectangle<float> face;          // printed area inside the trim line
        float cornerRadius = 14.0f;

        juce::Point<float> screws[4];
        float screwRadius = 6.0f;
        juce::Rectangle<float> earSlots[4];   // oval rack slots, two per ear

        juce::Rectangle<float> meter;         // bronze bezel
        juce::Rectangle<float> meterCard;     // amber scale card
        juce::Point<float> meterScrews[4];
        float meterScrewRadius = 3.2f;

        juce::Point<float> lamp;              // amber jewel (S.C. LINK)
        float lampRadius = 10.0f;
        juce::Point<float> powerLamp;         // red jewel
        float powerLampRadius = 9.5f;

        juce::Rectangle<float> linkToggle;    // hex bat switch
        juce::Point<float> linkCaptionPos;    // "S.C. LINK"
        juce::Point<float> linkOffLabelPos;   // "OFF"
        juce::Point<float> linkOnLabelPos;    // "ON"
        juce::Rectangle<float> powerToggle;
        juce::Point<float> powerCaptionPos;   // "POWER"
        juce::Point<float> powerOnLabelPos;   // "ON"

        juce::Point<float> brandMark;         // diamond monogram centre
        float brandMarkHalfW = 44.0f;
        float brandMarkHalfH = 26.0f;
        juce::String brandMarkText;           // "E·B"
        juce::Rectangle<float> brandName;     // "EMBER 175"
        juce::Rectangle<float> brandSub;      // "TUBE COMPRESSOR"
        float hairlineY = 240.0f;
        float hairlineLeft[2] { 97.0f, 524.0f };
        float hairlineRight[2] { 698.0f, 1106.0f };

        KnobSpec lowHigh, input, impedance, vernier, attack, output, mix, release;

        KnobSpec* knobFor (int index) noexcept;
        const KnobSpec* knobFor (int index) const noexcept;
        static constexpr int numKnobs = 8;
    };

    PanelLayout makeLayout();
}
