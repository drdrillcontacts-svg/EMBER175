#pragma once

/*
    Geometry for the EMBER 175-B front panel, in design units.

    Everything is described in a 1200 x 600 design space so that the renderer,
    the editor and the offline preview tool all agree. The editor maps that
    space onto the window with a uniform scale, so the panel keeps its hardware
    proportions at any window size.
*/

#include <juce_graphics/juce_graphics.h>

namespace ember::ui
{
    constexpr float kDesignWidth = 1200.0f;
    constexpr float kDesignHeight = 600.0f;

    enum class KnobStyle
    {
        large,      // input / output attenuator, 62 mm bakelite
        medium,     // mix / headroom
        small       // verniers, attack, release
    };

    struct KnobSpec
    {
        juce::String label;
        juce::String subLabel;
        KnobStyle style = KnobStyle::small;
        juce::Point<float> centre;
        float radius = 22.0f;          // bakelite radius
        float ringRadius = 0.0f;       // printed scale ring
        float angleMin = 0.0f;         // radians, measured from 12 o'clock
        float angleMax = 0.0f;
        int tickCount = 0;
        juce::StringArray tickLabels;
        bool showNumbers = false;
        bool labelBelow = false;

        juce::Rectangle<float> bounds() const
        {
            return { centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f };
        }
    };

    struct PanelLayout
    {
        juce::Rectangle<float> chassis;
        juce::Rectangle<float> earLeft, earRight;
        juce::Rectangle<float> face;
        juce::Rectangle<float> strip;

        juce::Rectangle<float> meter;        // including bezel
        juce::Rectangle<float> meterFace;    // printed scale
        juce::Rectangle<float> brandPlate;
        juce::Rectangle<float> readout;      // strip display

        juce::Point<float> screws[4];

        juce::Rectangle<float> xlrInput[2];
        juce::Rectangle<float> xlrOutput[2];
        juce::Point<float> xlrInputLabel;
        juce::Point<float> xlrOutputLabel;

        KnobSpec input, inputTrim, grTrim, attack, release, output, outputTrim, mix, headroom;

        juce::Rectangle<float> power;        // power / bypass toggle
        juce::Point<float> powerLabel;

        juce::Rectangle<float> linkButton, vernierButton, meterButton;
        juce::Point<float> footerText;

        KnobSpec* knobFor (int index) noexcept;
        static constexpr int numKnobs = 9;
    };

    PanelLayout makeLayout();
}
