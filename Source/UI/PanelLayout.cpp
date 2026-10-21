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

    l.meter = { 556.0f, 64.0f, 250.0f, 192.0f };
    l.meterFace = l.meter.reduced (9.0f);
    // (needle pivot is inside the face, see PanelRenderer)
    l.brandPlate = { 556.0f, 264.0f, 250.0f, 56.0f };
    l.readout = { 792.0f, 504.0f, 300.0f, 66.0f };

    l.xlrInput[0] = { 100.0f, 92.0f, 58.0f, 58.0f };
    l.xlrInput[1] = { 172.0f, 92.0f, 58.0f, 58.0f };
    l.xlrOutput[0] = { 100.0f, 250.0f, 58.0f, 58.0f };
    l.xlrOutput[1] = { 172.0f, 250.0f, 58.0f, 58.0f };
    l.xlrInputLabel = { 165.0f, 62.0f };
    l.xlrOutputLabel = { 165.0f, 220.0f };

    const float bigSweep = deg (150.0f);
    const float smallSweep = deg (135.0f);

    l.input.label = "INPUT";
    l.input.style = KnobStyle::large;
    l.input.centre = { 400.0f, 248.0f };
    l.input.radius = 60.0f;
    l.input.ringRadius = 90.0f;
    l.input.angleMin = -bigSweep;
    l.input.angleMax = bigSweep;
    l.input.tickCount = 21;
    l.input.tickLabels = numbers (0, 40, 2);
    l.input.showNumbers = true;

    l.output.label = "OUTPUT";
    l.output.style = KnobStyle::large;
    l.output.centre = { 936.0f, 318.0f };
    l.output.radius = 56.0f;
    l.output.ringRadius = 84.0f;
    l.output.angleMin = -bigSweep;
    l.output.angleMax = bigSweep;
    l.output.tickCount = 21;
    l.output.tickLabels = numbers (0, 40, 2);
    l.output.showNumbers = true;

    l.inputTrim.label = "INPUT VERNIER";
    l.inputTrim.style = KnobStyle::small;
    l.inputTrim.centre = { 206.0f, 366.0f };
    l.inputTrim.radius = 22.0f;
    l.inputTrim.ringRadius = 40.0f;
    l.inputTrim.angleMin = -smallSweep;
    l.inputTrim.angleMax = smallSweep;
    l.inputTrim.tickCount = 11;
    l.inputTrim.tickLabels = numbers (-2, 2, 1);
    l.inputTrim.labelBelow = true;

    l.grTrim.label = "G/R VERNIER";
    l.grTrim.style = KnobStyle::small;
    l.grTrim.centre = { 556.0f, 424.0f };
    l.grTrim.radius = 22.0f;
    l.grTrim.ringRadius = 40.0f;
    l.grTrim.angleMin = -smallSweep;
    l.grTrim.angleMax = smallSweep;
    l.grTrim.tickCount = 11;

    l.attack.label = "ATTACK";
    l.attack.style = KnobStyle::small;
    l.attack.centre = { 838.0f, 150.0f };
    l.attack.radius = 25.0f;
    l.attack.ringRadius = 42.0f;
    l.attack.angleMin = -smallSweep;
    l.attack.angleMax = smallSweep;
    l.attack.tickCount = 11;

    l.release.label = "RELEASE";
    l.release.style = KnobStyle::small;
    l.release.centre = { 1042.0f, 150.0f };
    l.release.radius = 25.0f;
    l.release.ringRadius = 42.0f;
    l.release.angleMin = -smallSweep;
    l.release.angleMax = smallSweep;
    l.release.tickCount = 11;

    l.outputTrim.label = "VERNIER";
    l.outputTrim.style = KnobStyle::small;
    l.outputTrim.centre = { 762.0f, 372.0f };
    l.outputTrim.radius = 20.0f;
    l.outputTrim.ringRadius = 36.0f;
    l.outputTrim.angleMin = -smallSweep;
    l.outputTrim.angleMax = smallSweep;
    l.outputTrim.tickCount = 11;
    l.outputTrim.labelBelow = true;

    l.mix.label = "PARALLEL MIX";
    l.mix.style = KnobStyle::medium;
    l.mix.centre = { 152.0f, 522.0f };
    l.mix.radius = 22.0f;
    l.mix.ringRadius = 32.0f;
    l.mix.angleMin = -smallSweep;
    l.mix.angleMax = smallSweep;
    l.mix.tickCount = 11;
    l.mix.labelBelow = true;

    l.headroom.label = "HEADROOM";
    l.headroom.style = KnobStyle::medium;
    l.headroom.centre = { 274.0f, 522.0f };
    l.headroom.radius = 22.0f;
    l.headroom.ringRadius = 32.0f;
    l.headroom.angleMin = -smallSweep;
    l.headroom.angleMax = smallSweep;
    l.headroom.tickCount = 11;
    l.headroom.labelBelow = true;

    l.linkButton = { 330.0f, 512.0f, 128.0f, 46.0f };
    l.vernierButton = { 470.0f, 512.0f, 146.0f, 46.0f };
    l.meterButton = { 628.0f, 512.0f, 150.0f, 46.0f };

    l.power = { 1076.0f, 330.0f, 44.0f, 66.0f };
    l.powerLabel = { 1098.0f, 308.0f };

    l.footerText = { 792.0f, 578.0f };

    return l;
}
}
