#include "PanelLayout.h"

namespace ember::ui
{
namespace
{
    constexpr float deg (float d) { return d * juce::MathConstants<float>::pi / 180.0f; }

    juce::StringArray numbers (int first, int last, int step)
    {
        juce::StringArray a;
        for (int v = first; v <= last; v += step)
            a.add (juce::String (v));
        return a;
    }
}

KnobSpec* PanelLayout::knobFor (int index) noexcept
{
    switch (index)
    {
        case 0: return &input;
        case 1: return &output;
        case 2: return &attack;
        case 3: return &release;
        case 4: return &inputTrim;
        case 5: return &outputTrim;
        case 6: return &grTrim;
        case 7: return &mix;
        case 8: return &headroom;
        default: break;
    }
    return nullptr;
}

PanelLayout makeLayout()
{
    PanelLayout l;

    l.chassis = { 0.0f, 0.0f, kDesignWidth, kDesignHeight };
    l.earLeft = { 0.0f, 0.0f, 74.0f, kDesignHeight };
    l.earRight = { kDesignWidth - 74.0f, 0.0f, 74.0f, kDesignHeight };
    l.face = { 74.0f, 8.0f, kDesignWidth - 148.0f, 472.0f };
    l.strip = { 74.0f, 486.0f, kDesignWidth - 148.0f, 106.0f };

    l.screws[0] = { 94.0f, 26.0f };
    l.screws[1] = { kDesignWidth - 94.0f, 26.0f };
    l.screws[2] = { 94.0f, 462.0f };
    l.screws[3] = { kDesignWidth - 94.0f, 462.0f };

    // meter sits left of centre, as on the rack unit, so that the ATTACK knob
    // has room to its right
    l.meter = { 516.0f, 58.0f, 244.0f, 186.0f };
    l.meterFace = l.meter.reduced (9.0f);
    l.brandPlate = { 516.0f, 252.0f, 244.0f, 52.0f };
    l.readout = { 800.0f, 500.0f, 292.0f, 70.0f };

    l.xlrInput[0] = { 98.0f, 90.0f, 58.0f, 58.0f };
    l.xlrInput[1] = { 170.0f, 90.0f, 58.0f, 58.0f };
    l.xlrOutput[0] = { 98.0f, 248.0f, 58.0f, 58.0f };
    l.xlrOutput[1] = { 170.0f, 248.0f, 58.0f, 58.0f };
    l.xlrInputLabel = { 163.0f, 60.0f };
    l.xlrOutputLabel = { 163.0f, 218.0f };

    const float bigSweep = deg (150.0f);
    const float smallSweep = deg (135.0f);

    // ---- INPUT: the big stepped attenuator ------------------------------
    l.input.label = "INPUT";
    l.input.style = KnobStyle::large;
    l.input.centre = { 396.0f, 250.0f };
    l.input.radius = 60.0f;
    l.input.ringRadius = 92.0f;
    l.input.angleMin = -bigSweep;
    l.input.angleMax = bigSweep;
    l.input.tickCount = 21;
    l.input.tickLabels = numbers (0, 40, 2);
    l.input.showNumbers = true;

    // ---- OUTPUT: the other big stepped attenuator -----------------------
    l.output.label = "OUTPUT";
    l.output.style = KnobStyle::large;
    l.output.centre = { 944.0f, 344.0f };
    l.output.radius = 56.0f;
    l.output.ringRadius = 86.0f;
    l.output.angleMin = -bigSweep;
    l.output.angleMax = bigSweep;
    l.output.tickCount = 21;
    l.output.tickLabels = numbers (0, 40, 2);
    l.output.showNumbers = true;

    // ---- attack and release: proper bakelite knobs, not trim pots --------
    l.attack.label = "ATTACK";
    l.attack.style = KnobStyle::medium;
    l.attack.centre = { 852.0f, 152.0f };
    l.attack.radius = 32.0f;
    l.attack.ringRadius = 58.0f;
    l.attack.angleMin = -smallSweep;
    l.attack.angleMax = smallSweep;
    l.attack.tickCount = 11;

    l.release.label = "RELEASE";
    l.release.style = KnobStyle::medium;
    l.release.centre = { 1046.0f, 152.0f };
    l.release.radius = 32.0f;
    l.release.ringRadius = 56.0f;
    l.release.angleMin = -smallSweep;
    l.release.angleMax = smallSweep;
    l.release.tickCount = 11;

    // ---- verniers -------------------------------------------------------
    l.inputTrim.label = "INPUT VERNIER";
    l.inputTrim.style = KnobStyle::medium;
    l.inputTrim.centre = { 212.0f, 380.0f };
    l.inputTrim.radius = 27.0f;
    l.inputTrim.ringRadius = 50.0f;
    l.inputTrim.angleMin = -smallSweep;
    l.inputTrim.angleMax = smallSweep;
    l.inputTrim.tickCount = 11;
    l.inputTrim.tickLabels = numbers (-2, 2, 1);
    l.inputTrim.labelBelow = true;

    l.grTrim.label = "G/R VERNIER";
    l.grTrim.style = KnobStyle::medium;
    l.grTrim.centre = { 570.0f, 400.0f };
    l.grTrim.radius = 27.0f;
    l.grTrim.ringRadius = 48.0f;
    l.grTrim.angleMin = -smallSweep;
    l.grTrim.angleMax = smallSweep;
    l.grTrim.tickCount = 11;

    l.outputTrim.label = "VERNIER";
    l.outputTrim.style = KnobStyle::medium;
    l.outputTrim.centre = { 742.0f, 372.0f };
    l.outputTrim.radius = 27.0f;
    l.outputTrim.ringRadius = 48.0f;
    l.outputTrim.angleMin = -smallSweep;
    l.outputTrim.angleMax = smallSweep;
    l.outputTrim.tickCount = 11;
    l.outputTrim.labelBelow = true;

    // ---- extended controls ---------------------------------------------
    l.mix.label = "PARALLEL MIX";
    l.mix.style = KnobStyle::medium;
    l.mix.centre = { 152.0f, 528.0f };
    l.mix.radius = 23.0f;
    l.mix.ringRadius = 33.0f;
    l.mix.angleMin = -smallSweep;
    l.mix.angleMax = smallSweep;
    l.mix.tickCount = 11;
    l.mix.labelBelow = true;

    l.headroom.label = "HEADROOM";
    l.headroom.style = KnobStyle::medium;
    l.headroom.centre = { 268.0f, 528.0f };
    l.headroom.radius = 23.0f;
    l.headroom.ringRadius = 33.0f;
    l.headroom.angleMin = -smallSweep;
    l.headroom.angleMax = smallSweep;
    l.headroom.tickCount = 11;
    l.headroom.labelBelow = true;

    l.linkButton = { 336.0f, 508.0f, 122.0f, 42.0f };
    l.vernierButton = { 470.0f, 508.0f, 150.0f, 42.0f };
    l.meterButton = { 632.0f, 508.0f, 148.0f, 42.0f };

    l.power = { 1082.0f, 330.0f, 44.0f, 66.0f };
    l.powerLabel = { 1104.0f, 308.0f };

    l.footerText = { 800.0f, 578.0f };

    return l;
}
}
