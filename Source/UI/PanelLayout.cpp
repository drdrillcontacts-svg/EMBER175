#include "PanelLayout.h"

namespace ember::ui
{
namespace
{
    constexpr float deg (float d) { return juce::degreesToRadians (d); }

    juce::StringArray numbers (float first, float last, float step)
    {
        juce::StringArray a;
        for (float v = first; v <= last + 0.001f; v += step)
            a.add ((v > 0.0f ? "+" : "") + juce::String (juce::roundToInt (v)));
        return a;
    }

    /** Evenly spaced marks over the full travel. */
    std::vector<ScaleMark> spread (const juce::StringArray& texts)
    {
        std::vector<ScaleMark> m;
        const int n = texts.size();
        for (int i = 0; i < n; ++i)
            m.push_back ({ n > 1 ? static_cast<float> (i) / static_cast<float> (n - 1) : 0.0f, texts[i] });
        return m;
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
        default: return nullptr;
    }
}

const KnobSpec* PanelLayout::knobFor (int index) const noexcept
{
    return const_cast<PanelLayout*> (this)->knobFor (index);
}

PanelLayout makeLayout()
{
    PanelLayout l;

    l.panel = { 0.0f, 0.0f, kDesignWidth, kDesignHeight };
    l.cornerRadius = 13.0f;

    // rack ears either side of the wine case
    const float e = kEarWidth;
    l.ears[0] = { 0.0f, 3.0f, e - 3.0f, kDesignHeight - 6.0f };
    l.ears[1] = { kDesignWidth - e + 3.0f, 3.0f, e - 3.0f, kDesignHeight - 6.0f };
    l.panelCase = { e, 0.0f, kDesignWidth - 2.0f * e, kDesignHeight };
    l.face = l.panelCase.reduced (6.0f);

    l.earSlots[0] = { 9.0f, 40.0f, 30.0f, 22.0f };
    l.earSlots[1] = { 9.0f, kDesignHeight - 62.0f, 30.0f, 22.0f };
    l.earSlots[2] = { kDesignWidth - 39.0f, 40.0f, 30.0f, 22.0f };
    l.earSlots[3] = { kDesignWidth - 39.0f, kDesignHeight - 62.0f, 30.0f, 22.0f };

    l.screws[0] = { e + 7.0f, 12.0f };
    l.screws[1] = { kDesignWidth - e - 7.0f, 12.0f };
    l.screws[2] = { e + 7.0f, kDesignHeight - 12.0f };
    l.screws[3] = { kDesignWidth - e - 7.0f, kDesignHeight - 12.0f };

    // ------------------------------------------------------------------ meter
    l.meter = { 492.0f, 19.0f, 248.0f, 134.0f };
    l.meterCard = l.meter.reduced (14.0f, 13.0f);
    {
        const auto& m = l.meter;
        const float i = 5.0f;
        l.meterScrews[0] = { m.getX() + i, m.getY() + i };
        l.meterScrews[1] = { m.getRight() - i, m.getY() + i };
        l.meterScrews[2] = { m.getX() + i, m.getBottom() - i };
        l.meterScrews[3] = { m.getRight() - i, m.getBottom() - i };
    }

    // --------------------------------------------------------------- switches
    l.linkToggle = { 66.0f, 178.0f, 30.0f, 30.0f };
    l.linkCaptionPos = { 81.0f, 134.0f };
    l.linkOffLabelPos = { 81.0f, 160.0f };
    l.linkOnLabelPos = { 81.0f, 224.0f };
    l.lamp = { 140.0f, 192.0f };
    l.lampRadius = 9.5f;

    l.powerToggle = { 1153.0f, 180.0f, 30.0f, 30.0f };
    l.powerCaptionPos = { 1108.0f, 160.0f };
    l.powerOnLabelPos = { 1169.0f, 160.0f };
    l.powerLamp = { 1110.0f, 196.0f };
    l.powerLampRadius = 9.5f;

    // ----------------------------------------------------------------- brand
    l.brandMark = { 611.0f, 181.0f };
    l.brandMarkHalfW = 46.0f;
    l.brandMarkHalfH = 27.0f;
    l.brandMarkText = "E.B";                     // the dot is drawn as a middle dot
    l.brandName = { 517.0f, 218.0f, 186.0f, 22.0f };
    l.brandSub = { 517.0f, 241.0f, 186.0f, 14.0f };
    l.hairlineY = 241.0f;
    l.hairlineLeft[0] = 97.0f;   l.hairlineLeft[1] = 520.0f;
    l.hairlineRight[0] = 702.0f; l.hairlineRight[1] = 1106.0f;

    // ------------------------------------------------------------ LOW / HI
    {
        auto& k = l.lowHigh;
        k.caption.clear();
        k.style = KnobStyle::small;
        k.centre = { 98.0f, 82.0f };
        k.skirtRadius = 24.0f;
        k.capRadius = 19.0f;
        k.tickRadius = 34.0f;
        k.labelRadius = 42.0f;
        k.angleMin = deg (-58.0f);
        k.angleMax = deg (58.0f);
        k.tickCount = 13;
        k.leftLabel = "LOW";
        k.rightLabel = "HI";
        k.leftLabelPos = { 64.0f, 35.0f };
        k.rightLabelPos = { 136.0f, 34.0f };
    }

    // --------------------------------------------------------------- INPUT
    {
        auto& k = l.input;
        k.caption = "INPUT";
        k.captionPos = { 253.0f, 23.0f };
        k.style = KnobStyle::big;
        k.centre = { 253.0f, 113.0f };
        k.skirtRadius = 45.0f;
        k.capRadius = 36.0f;
        k.tickRadius = 56.0f;
        k.labelRadius = 68.0f;
        k.labelHeight = 9.5f;
        k.angleMin = deg (-140.0f);
        k.angleMax = deg (140.0f);
        k.tickCount = 21;
        k.marks = spread (numbers (-20.0f, 20.0f, 4.0f));   // 11 printed numbers
    }

    // ----------------------------------------------------------- IMPEDANCE
    {
        auto& k = l.impedance;
        k.caption = "IMPEDANCE";
        k.captionPos = { 413.0f, 26.0f };
        k.style = KnobStyle::small;
        k.centre = { 413.0f, 80.0f };
        k.skirtRadius = 24.0f;
        k.capRadius = 19.0f;
        k.tickRadius = 34.0f;
        k.labelRadius = 42.0f;
        k.angleMin = deg (-58.0f);
        k.angleMax = deg (58.0f);
        k.tickCount = 13;
        k.leftLabel = "600";
        k.rightLabel = "300";
        k.leftLabelPos = { 372.0f, 40.0f };
        k.rightLabelPos = { 454.0f, 40.0f };
    }

    // ------------------------------------------------------------- VERNIER
    {
        auto& k = l.vernier;
        k.caption = "VERNIER";
        k.captionPos = { 411.0f, 129.0f };
        k.style = KnobStyle::small;
        k.centre = { 409.0f, 180.0f };
        k.skirtRadius = 24.0f;
        k.capRadius = 19.0f;
        k.tickRadius = 33.0f;
        k.labelRadius = 41.0f;
        k.angleMin = deg (-140.0f);
        k.angleMax = deg (140.0f);
        k.tickCount = 11;
        k.marks = { { 0.5f, "0" } };
        k.leftLabel = "-5";
        k.rightLabel = "+5";
        k.leftLabelPos = { 366.0f, 160.0f };
        k.rightLabelPos = { 458.0f, 160.0f };
    }

    // -------------------------------------------------------------- ATTACK
    {
        auto& k = l.attack;
        k.caption = "ATTACK";
        k.captionPos = { 814.0f, 18.0f };
        k.style = KnobStyle::small;
        k.centre = { 814.0f, 72.0f };
        k.skirtRadius = 24.0f;
        k.capRadius = 19.0f;
        k.tickRadius = 33.0f;
        k.labelRadius = 41.0f;
        k.labelHeight = 8.8f;
        k.angleMin = deg (-140.0f);
        k.angleMax = deg (140.0f);
        k.tickCount = 13;
        k.marks = { { 0.16f, ".1" }, { 0.50f, ".3" }, { 0.84f, "1" } };
        k.offLabel = "OFF";
        k.offLabelPos = { 776.0f, 95.0f };
    }

    // -------------------------------------------------------------- OUTPUT
    {
        auto& k = l.output;
        k.caption = "OUTPUT";
        k.captionPos = { 967.0f, 15.0f };
        k.style = KnobStyle::big;
        k.centre = { 967.0f, 112.0f };
        k.skirtRadius = 45.0f;
        k.capRadius = 36.0f;
        k.tickRadius = 56.0f;
        k.labelRadius = 68.0f;
        k.labelHeight = 9.5f;
        k.angleMin = deg (-140.0f);
        k.angleMax = deg (140.0f);
        k.tickCount = 21;
        k.marks = spread (numbers (-20.0f, 20.0f, 4.0f));
    }

    // ----------------------------------------------------------------- MIX
    {
        auto& k = l.mix;
        k.caption = "MIX";
        k.captionPos = { 809.0f, 116.0f };
        k.style = KnobStyle::small;
        k.centre = { 809.0f, 169.0f };
        k.skirtRadius = 24.0f;
        k.capRadius = 19.0f;
        k.tickRadius = 33.0f;
        k.labelRadius = 41.0f;
        k.angleMin = deg (-140.0f);
        k.angleMax = deg (140.0f);
        k.tickCount = 17;
        k.marks = spread (juce::StringArray { "0", "25", "50", "75", "100" });
    }

    // ------------------------------------------------------------- RELEASE
    {
        auto& k = l.release;
        k.caption = "RELEASE";
        k.captionPos = { 1121.0f, 18.0f };
        k.style = KnobStyle::small;
        k.centre = { 1121.0f, 72.0f };
        k.skirtRadius = 24.0f;
        k.capRadius = 19.0f;
        k.tickRadius = 33.0f;
        k.labelRadius = 41.0f;
        k.labelHeight = 9.5f;
        k.angleMin = deg (-140.0f);
        k.angleMax = deg (140.0f);
        k.tickCount = 13;
        k.marks = { { 0.16f, "30" }, { 0.50f, "120" }, { 0.84f, "500" } };
        k.offLabel.clear();   // the release dial has no OFF detent on this model
        k.offLabelPos = { 1098.0f, 104.0f };
    }

    // ---------------------------------------------------------------- offset
    // The interior was laid out in a 1200 wide space (measured from the
    // reference panel); the ears add 50 units on either side.
    {
        const float dx = kEarWidth;
        auto shift = [dx] (juce::Point<float>& p) { p.x += dx; };
        auto shiftRect = [dx] (juce::Rectangle<float>& r) { r.setX (r.getX() + dx); };

        shiftRect (l.meter);
        shiftRect (l.meterCard);
        for (auto& p : l.meterScrews) shift (p);

        shift (l.lamp);
        shift (l.powerLamp);
        shiftRect (l.linkToggle);
        shift (l.linkCaptionPos);
        shift (l.linkOffLabelPos);
        shift (l.linkOnLabelPos);
        shiftRect (l.powerToggle);
        shift (l.powerCaptionPos);
        shift (l.powerOnLabelPos);

        shift (l.brandMark);
        shiftRect (l.brandName);
        shiftRect (l.brandSub);
        l.hairlineLeft[0] += dx;
        l.hairlineLeft[1] += dx;
        l.hairlineRight[0] += dx;
        l.hairlineRight[1] += dx;

        for (int i = 0; i < PanelLayout::numKnobs; ++i)
        {
            auto* k = l.knobFor (i);
            shift (k->centre);
            shift (k->captionPos);
            shift (k->leftLabelPos);
            shift (k->rightLabelPos);
            shift (k->offLabelPos);
        }
    }

    return l;
}
} // namespace ember::ui
