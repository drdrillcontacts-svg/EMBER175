#include "PanelLayout.h"

namespace ember::ui
{
namespace
{
    constexpr float deg (float d) { return d * juce::MathConstants<float>::pi / 180.0f; }

    juce::StringArray numbers (float first, float last, float step, int decimals = 0)
    {
        juce::StringArray a;
        for (float v = first; v <= last + 0.001f; v += step)
            a.add (juce::String (v, decimals));
        return a;
    }
}

KnobSpec* PanelLayout::knobFor (int index) noexcept
{
    switch (index)
    {
        case 0: return &lowHigh;
        case 1: return &input;
        case 2: return &impedance;
        case 3: return &vernier;
        case 4: return &attack;
        case 5: return &output;
        case 6: return &mix;
        case 7: return &release;
        default: break;
    }
    return nullptr;
}

PanelLayout makeLayout()
{
    PanelLayout l;

    l.panel = { 0.0f, 0.0f, kDesignWidth, kDesignHeight };
    l.face  = { 10.0f, 8.0f, kDesignWidth - 20.0f, kDesignHeight - 16.0f };

    // ---- meter -----------------------------------------------------------
    l.meter     = { 500.0f, 40.0f, 284.0f, 146.0f };
    l.meterFace = l.meter.reduced (8.0f);

    // ---- brand block -----------------------------------------------------
    l.brandLogo = { 642.0f, 232.0f };
    l.brandLogoRadius = 21.0f;
    l.brandName = { 530.0f, 262.0f, 224.0f, 22.0f };
    l.brandSub  = { 530.0f, 284.0f, 224.0f, 14.0f };

    // ---- left group ------------------------------------------------------
    l.lowHigh.label = "";
    l.lowHigh.style = KnobStyle::small;
    l.lowHigh.centre = { 78.0f, 92.0f };
    l.lowHigh.radius = 16.0f;
    l.lowHigh.ringRadius = 27.0f;
    l.lowHigh.angleMin = deg (-38.0f);
    l.lowHigh.angleMax = deg (38.0f);
    l.lowHigh.tickCount = 3;
    l.lowHigh.leftLabel = "LOW";
    l.lowHigh.rightLabel = "HI";
    l.lowHigh.captionAbove = false;

    l.input.label = "INPUT";
    l.input.style = KnobStyle::big;
    l.input.centre = { 262.0f, 138.0f };
    l.input.radius = 52.0f;
    l.input.ringRadius = 74.0f;
    l.input.angleMin = deg (-145.0f);
    l.input.angleMax = deg (145.0f);
    l.input.tickCount = 17;
    l.input.tickLabels = numbers (0.0f, 40.0f, 5.0f);     // 9 labels, 17 ticks
    l.input.captionAbove = true;

    l.impedance.label = "IMPEDANCE";
    l.impedance.style = KnobStyle::small;
    l.impedance.centre = { 396.0f, 96.0f };
    l.impedance.radius = 17.0f;
    l.impedance.ringRadius = 29.0f;
    l.impedance.angleMin = deg (-38.0f);
    l.impedance.angleMax = deg (38.0f);
    l.impedance.tickCount = 3;
    l.impedance.leftLabel = "600";
    l.impedance.rightLabel = "300";
    l.impedance.captionAbove = true;

    l.vernier.label = "VERNIER";
    l.vernier.style = KnobStyle::medium;
    l.vernier.centre = { 396.0f, 216.0f };
    l.vernier.radius = 20.0f;
    l.vernier.ringRadius = 32.0f;
    l.vernier.angleMin = deg (-140.0f);
    l.vernier.angleMax = deg (140.0f);
    l.vernier.tickCount = 11;
    l.vernier.leftLabel = "-2";
    l.vernier.rightLabel = "+2";
    l.vernier.captionAbove = true;

    // ---- right group -----------------------------------------------------
    l.attack.style = KnobStyle::medium;
    l.attack.centre = { 852.0f, 98.0f };
    l.attack.radius = 20.0f;
    l.attack.ringRadius = 32.0f;
    l.attack.angleMin = deg (-140.0f);
    l.attack.angleMax = deg (140.0f);
    l.attack.tickCount = 11;
    l.attack.tickCount = 8;
    l.attack.tickLabels = juce::StringArray { "1", "2", "3", "4", "5", "6", "7", "8" };
    l.attack.label = "ATTACK";
    l.attack.offLabel = "OFF";
    l.attack.captionAbove = true;

    l.output.label = "OUTPUT";
    l.output.style = KnobStyle::big;
    l.output.centre = { 1006.0f, 146.0f };
    l.output.radius = 50.0f;
    l.output.ringRadius = 72.0f;
    l.output.angleMin = deg (-145.0f);
    l.output.angleMax = deg (145.0f);
    l.output.tickCount = 17;
    l.output.tickLabels = numbers (0.0f, 40.0f, 5.0f);    // 9 labels, 17 ticks
    l.output.captionAbove = true;

    l.mix.style = KnobStyle::medium;
    l.mix.centre = { 862.0f, 226.0f };
    l.mix.radius = 20.0f;
    l.mix.ringRadius = 32.0f;
    l.mix.angleMin = deg (-140.0f);
    l.mix.angleMax = deg (140.0f);
    l.mix.tickCount = 9;
    l.mix.tickLabels = juce::StringArray { "0", "25", "50", "75", "100" };
    l.mix.captionAbove = true;
    l.mix.label = "MIX";

    l.release.style = KnobStyle::medium;
    l.release.centre = { 1140.0f, 98.0f };
    l.release.radius = 20.0f;
    l.release.ringRadius = 32.0f;
    l.release.angleMin = deg (-140.0f);
    l.release.angleMax = deg (140.0f);
    l.release.tickCount = 11;
    l.release.tickCount = 8;
    l.release.tickLabels = juce::StringArray { "1", "2", "3", "4", "5", "6", "7", "8" };
    l.release.label = "RELEASE";
    l.release.offLabel = "OFF";
    l.release.captionAbove = true;

    // ---- switches and lamps ---------------------------------------------
    l.linkToggle = { 66.0f, 230.0f, 24.0f, 40.0f };
    l.linkOffLabel = { 78.0f, 206.0f };
    l.linkOnLabel  = { 78.0f, 278.0f };
    l.lamp = { 152.0f, 250.0f };
    l.lampRadius = 7.5f;

    l.powerToggle = { 1118.0f, 250.0f, 24.0f, 40.0f };
    l.powerOnLabel = { 1130.0f, 298.0f };
    l.powerLamp = { 1064.0f, 270.0f };
    l.powerLampRadius = 7.0f;

    l.statusLine = { 700.0f, 330.0f, 470.0f, 16.0f };

    return l;
}
}
