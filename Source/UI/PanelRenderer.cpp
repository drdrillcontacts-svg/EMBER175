#include "PanelRenderer.h"

namespace ember::ui
{
namespace
{
    constexpr float kPi = juce::MathConstants<float>::pi;

    // ------------------------------------------------------------------ palette
    const juce::Colour cFace        { 0xff4a3a3d };
    const juce::Colour cFaceDeep    { 0xff35262e };
    const juce::Colour cFaceLight   { 0xff5b4650 };
    const juce::Colour cPrint       { 0xffe7dbc6 };
    const juce::Colour cPrintMuted  { 0xffc3b39c };
    const juce::Colour cBrass       { 0xffc9a15c };
    const juce::Colour cSteel       { 0xff9ba0a6 };
    const juce::Colour cSteelDark   { 0xff5f646a };
    const juce::Colour cBakelite    { 0xff171310 };
    const juce::Colour cMeterFace   { 0xffe8dbb8 };
    const juce::Colour cMeterInk    { 0xff3a2a20 };
    const juce::Colour cNeedle      { 0xffc8452c };

    void text (juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r,
               float height, juce::Colour colour, juce::Justification just = juce::Justification::centred,
               bool bold = true, float kerning = 0.16f)
    {
        g.setColour (colour);
        g.setFont (PanelRenderer::panelFont (height, bold, kerning));
        g.drawText (s, r, just, false);
    }

    void screw (juce::Graphics& g, juce::Point<float> p, float r, float angle)
    {
        // recessed slot head
        g.setColour (juce::Colour (0x66000000));
        g.fillEllipse (p.x - r - 1.2f, p.y - r - 1.2f, (r + 1.2f) * 2.0f, (r + 1.2f) * 2.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb9bcc0), p.x - r * 0.5f, p.y - r * 0.6f,
                                                 juce::Colour (0xff585c61), p.x + r * 0.6f, p.y + r * 0.7f, true));
        g.fillEllipse (p.x - r, p.y - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colour (0x99403f3e));
        g.drawEllipse (p.x - r, p.y - r, r * 2.0f, r * 2.0f, 0.8f);
        g.setColour (juce::Colour (0xaa2a2622));
        const auto dir = juce::Point<float> (std::cos (angle), std::sin (angle)) * (r * 0.86f);
        g.drawLine ({ p.x - dir.x, p.y - dir.y, p.x + dir.x, p.y + dir.y }, 1.6f);
    }

    void brushedMetal (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour hi, juce::Colour lo, bool horizontal)
    {
        g.setGradientFill (horizontal
            ? juce::ColourGradient (hi, r.getX(), r.getY(), lo, r.getX(), r.getBottom(), false)
            : juce::ColourGradient (hi, r.getX(), r.getY(), lo, r.getRight(), r.getY(), false));
        g.fillRect (r);

        juce::Random rng (0x1758);
        for (int i = 0; i < 220; ++i)
        {
            const float t = rng.nextFloat();
            const float a = 0.02f + rng.nextFloat() * 0.05f;
            g.setColour (rng.nextBool() ? juce::Colours::white.withAlpha (a)
                                        : juce::Colours::black.withAlpha (a));
            if (horizontal)
                g.fillRect (r.getX(), r.getY() + t * r.getHeight(), r.getWidth(), 1.0f);
            else
                g.fillRect (r.getX() + t * r.getWidth(), r.getY(), 1.0f, r.getHeight());
        }
    }

    void weatheredPanel (juce::Graphics& g, juce::Rectangle<float> r)
    {
        g.setGradientFill (juce::ColourGradient (cFaceLight, r.getCentreX(), r.getY(),
                                                 cFaceDeep, r.getCentreX(), r.getBottom(), false));
        g.fillRect (r);

        // mottled anodising
        juce::Random rng (0x175B);
        const int dots = static_cast<int> (r.getWidth() * r.getHeight() / 130.0f);
        for (int i = 0; i < dots; ++i)
        {
            const float x = r.getX() + rng.nextFloat() * r.getWidth();
            const float y = r.getY() + rng.nextFloat() * r.getHeight();
            const float s = 0.6f + rng.nextFloat() * 1.8f;
            const float a = 0.012f + rng.nextFloat() * 0.035f;
            g.setColour (rng.nextBool() ? juce::Colours::white.withAlpha (a)
                                        : juce::Colours::black.withAlpha (a * 1.4f));
            g.fillEllipse (x, y, s, s);
        }

        // fine surface scratches
        for (int i = 0; i < 90; ++i)
        {
            const float x = r.getX() + rng.nextFloat() * r.getWidth();
            const float y = r.getY() + rng.nextFloat() * r.getHeight();
            const float len = 6.0f + rng.nextFloat() * 40.0f;
            const float ang = (rng.nextFloat() - 0.5f) * 0.35f;
            g.setColour (juce::Colours::white.withAlpha (0.02f + rng.nextFloat() * 0.04f));
            g.drawLine ({ x, y, x + std::cos (ang) * len, y + std::sin (ang) * len }, 0.7f);
        }

        // vignette
        juce::ColourGradient vign (juce::Colours::transparentBlack, r.getCentreX(), r.getCentreY(),
                                   juce::Colours::black.withAlpha (0.42f), r.getX(), r.getY(), true);
        g.setGradientFill (vign);
        g.fillRect (r);

        // edges
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.drawRect (r, 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawRect (r.reduced (2.5f), 1.0f);
    }

    void xlr (juce::Graphics& g, juce::Rectangle<float> r)
    {
        const auto c = r.getCentre();
        const float rad = r.getWidth() * 0.5f;

        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (r.translated (0.0f, 1.5f).expanded (1.4f));

        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffcfd3d7), c.x - rad * 0.7f, c.y - rad * 0.8f,
                                                 juce::Colour (0xff6a6e73), c.x + rad * 0.7f, c.y + rad * 0.8f, true));
        g.fillEllipse (r);
        g.setColour (juce::Colour (0xff2d2f31));
        g.drawEllipse (r, 1.2f);

        // recess
        auto inner = r.reduced (r.getWidth() * 0.13f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0b0b0c), inner.getCentreX(), inner.getY(),
                                                 juce::Colour (0xff2a2b2d), inner.getCentreX(), inner.getBottom(), false));
        g.fillEllipse (inner);

        // three pins
        for (int i = 0; i < 3; ++i)
        {
            const float a = -kPi * 0.5f + static_cast<float> (i) * 2.0f * kPi / 3.0f;
            const auto p = inner.getCentre() + juce::Point<float> (std::cos (a), std::sin (a)) * (inner.getWidth() * 0.26f);
            g.setColour (juce::Colour (0xff8b8172));
            g.fillEllipse (p.x - 2.6f, p.y - 2.6f, 5.2f, 5.2f);
            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.drawEllipse (p.x - 2.6f, p.y - 2.6f, 5.2f, 5.2f, 0.7f);
        }

        // catch tab
        g.setColour (juce::Colour (0xff5d6166));
        g.fillRoundedRectangle (c.x - rad * 0.16f, r.getY() + rad * 0.80f, rad * 0.32f, rad * 0.22f, 2.0f);
    }

    void scaleRing (juce::Graphics& g, const KnobSpec& k)
    {
        const auto c = k.centre;
        const float ring = k.ringRadius;
        const float tickOuter = ring;
        const float tickInner = ring - (k.style == KnobStyle::large ? 9.0f : 6.0f);

        // faint engraved circle
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.drawEllipse (c.x - ring - 1.0f, c.y - ring - 1.0f, (ring + 1.0f) * 2.0f, (ring + 1.0f) * 2.0f, 0.8f);
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawEllipse (c.x - ring, c.y - ring, ring * 2.0f, ring * 2.0f, 0.8f);

        for (int i = 0; i < k.tickCount; ++i)
        {
            const float t = k.tickCount > 1 ? static_cast<float> (i) / static_cast<float> (k.tickCount - 1) : 0.0f;
            const float a = k.angleMin + t * (k.angleMax - k.angleMin);
            const auto dir = juce::Point<float> (std::sin (a), -std::cos (a));
            const bool major = (i % 2 == 0);
            g.setColour (cPrint.withAlpha (major ? 0.92f : 0.55f));
            g.drawLine ({ c + dir * (major ? tickInner : tickInner + 2.5f), c + dir * tickOuter },
                        major ? 1.6f : 1.0f);
        }

        if (! k.showNumbers)
            return;

        const float numberRadius = ring + (k.style == KnobStyle::large ? 11.0f : 10.0f);
        g.setFont (PanelRenderer::panelFont (k.style == KnobStyle::large ? 12.5f : 9.0f, true, 0.02f));
        g.setColour (cPrint);

        const int count = k.tickLabels.size();
        for (int i = 0; i < count; ++i)
        {
            const float t = count > 1 ? static_cast<float> (i) / static_cast<float> (count - 1) : 0.0f;
            const float a = k.angleMin + t * (k.angleMax - k.angleMin);
            const auto p = c + juce::Point<float> (std::sin (a), -std::cos (a)) * numberRadius;
            g.saveState();
            g.addTransform (juce::AffineTransform::rotation (a, p.x, p.y));
            g.drawText (k.tickLabels[i], juce::Rectangle<float> (p.x - 16.0f, p.y - 7.0f, 32.0f, 14.0f),
                        juce::Justification::centred, false);
            g.restoreState();
        }
    }

    void knobLabel (juce::Graphics& g, const KnobSpec& k)
    {
        const float h = k.style == KnobStyle::large ? 15.0f : 10.5f;
        const float y = k.labelBelow
                            ? k.centre.y + k.ringRadius + (k.style == KnobStyle::large ? 30.0f : 22.0f)
                            : k.centre.y - k.ringRadius - (k.style == KnobStyle::large ? 34.0f : 26.0f);
        juce::Rectangle<float> r (k.centre.x - 90.0f, y, 180.0f, 18.0f);
        text (g, k.label, r, h, cPrint, juce::Justification::centred, true, 0.24f);
    }

    void meterFace (juce::Graphics& g, const PanelLayout& l)
    {
        auto bezel = l.meter;
        auto face = l.meterFace;

        // metal bezel
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (bezel.translated (0.0f, 2.0f), 6.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff9a8f7c), bezel.getX(), bezel.getY(),
                                                 juce::Colour (0xff3a332b), bezel.getRight(), bezel.getBottom(), false));
        g.fillRoundedRectangle (bezel, 6.0f);
        g.setColour (juce::Colour (0xff241f19));
        g.drawRoundedRectangle (bezel, 6.0f, 1.4f);
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawRoundedRectangle (bezel.reduced (1.5f), 5.0f, 0.8f);

        // cream scale card
        g.setGradientFill (juce::ColourGradient (cMeterFace.brighter (0.06f), face.getCentreX(), face.getY(),
                                                 cMeterFace.darker (0.16f), face.getCentreX(), face.getBottom(), false));
        g.fillRoundedRectangle (face, 3.0f);

        juce::Random rng (0x175C);
        for (int i = 0; i < 240; ++i)
        {
            g.setColour (juce::Colour (0xff7a5a33).withAlpha (0.02f + rng.nextFloat() * 0.05f));
            g.fillEllipse (face.getX() + rng.nextFloat() * face.getWidth(),
                           face.getY() + rng.nextFloat() * face.getHeight(), 1.5f, 1.5f);
        }

        // vignette to make the card look old
        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, face.getCentreX(), face.getCentreY(),
                                                 juce::Colour (0x33743f14), face.getX(), face.getBottom(), true));
        g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, face.getCentreX(), face.getCentreY(),
                                                 juce::Colour (0x30743f14), face.getX(), face.getY(), true));
        g.fillRoundedRectangle (face, 3.0f);

        const auto pivot = juce::Point<float> (face.getCentreX(), face.getY() + face.getHeight() * 0.94f);
        const float h = face.getHeight();
        const float len = h * 0.86f;
        const float sweep = juce::MathConstants<float>::pi * 0.30f;

        auto angleFor = [sweep] (float n) { return -sweep + juce::jlimit (0.0f, 1.0f, n) * sweep * 2.0f; };

        // VU arc and ticks
        {
            const float r = len * 0.93f;
            juce::Path arc;
            arc.addCentredArc (pivot.x, pivot.y, r, r, 0.0f, angleFor (0.0f) - 0.02f, angleFor (1.0f) + 0.02f, true);
            g.setColour (cMeterInk.withAlpha (0.85f));
            g.strokePath (arc, juce::PathStrokeType (1.1f));

            const int labelled[] = { -20, -10, -7, -5, -3, 0, 3 };
            for (int db = -20; db <= 3; ++db)
            {
                if (db < -10 && db % 5 != 0) continue;
                if (db > -10 && db != -7 && db != -5 && db != -3 && db != 0 && db != 3) continue;
                bool drawLabel = false;
                for (int v : labelled) if (v == db) drawLabel = true;
                const float n = (static_cast<float> (db) + 20.0f) / 23.0f;
                const auto a = angleFor (n);
                const auto dir = juce::Point<float> (std::sin (a), -std::cos (a));
                const bool major = db == 0 || db == -20 || db == -10;
                g.setColour (cMeterInk);
                g.drawLine ({ pivot + dir * (r - (major ? 9.0f : 6.0f)), pivot + dir * r }, major ? 1.6f : 1.0f);

                if (! drawLabel)
                    continue;

                const auto tp = pivot + dir * (r - 15.0f);
                g.setFont (PanelRenderer::panelFont (7.0f, true, 0.0f));
                g.setColour (cMeterInk);
                g.saveState();
                g.addTransform (juce::AffineTransform::rotation (a, tp.x, tp.y));
                g.drawText (db > 0 ? "+" + juce::String (db) : juce::String (db),
                            juce::Rectangle<float> (tp.x - 14.0f, tp.y - 6.0f, 28.0f, 12.0f),
                            juce::Justification::centred, false);
                g.restoreState();
            }

            text (g, "VU", { pivot.x + r * 0.42f, pivot.y - r * 0.80f, 24.0f, 11.0f },
                  7.5f, cMeterInk, juce::Justification::centred, true, 0.1f);
        }

        // gain reduction arc (red, above the VU scale)
        {
            const float r = len * 0.56f;
            juce::Path arc;
            arc.addCentredArc (pivot.x, pivot.y, r, r, 0.0f, angleFor (0.0f), angleFor (1.0f), true);
            g.setColour (cNeedle.withAlpha (0.85f));
            g.strokePath (arc, juce::PathStrokeType (1.4f));

            const int marks[] = { 0, 5, 10, 20 };
            for (int db : marks)
            {
                const float n = static_cast<float> (db) / 20.0f;
                const auto a = angleFor (n);
                const auto dir = juce::Point<float> (std::sin (a), -std::cos (a));
                g.setColour (cNeedle);
                g.drawLine ({ pivot + dir * (r - 5.0f), pivot + dir * (r + 3.0f) }, 1.2f);

                const auto tp = pivot + dir * (r - 9.5f);
                g.setFont (PanelRenderer::panelFont (6.6f, true, 0.0f));
                g.setColour (cNeedle.darker (0.25f));
                g.saveState();
                g.addTransform (juce::AffineTransform::rotation (a, tp.x, tp.y));
                g.drawText (juce::String (db), juce::Rectangle<float> (tp.x - 12.0f, tp.y - 5.0f, 24.0f, 10.0f),
                            juce::Justification::centred, false);
                g.restoreState();
            }

            text (g, "GR", { pivot.x - r * 1.02f, pivot.y - r * 0.36f, 26.0f, 11.0f },
                  7.0f, cNeedle.darker (0.2f), juce::Justification::centred, true, 0.1f);
        }

        // brand mark on the card
        {
            auto c = juce::Point<float> (face.getX() + 22.0f, face.getY() + 14.0f);
            juce::Path shield;
            shield.addTriangle (c.x - 4.0f, c.y - 4.0f, c.x + 4.0f, c.y - 4.0f, c.x, c.y + 5.0f);
            g.setColour (cMeterInk.withAlpha (0.5f));
            g.fillPath (shield);
            text (g, "EMBER", { face.getX() + 30.0f, face.getY() + 8.0f, 70.0f, 10.0f },
                  7.0f, cMeterInk.withAlpha (0.7f), juce::Justification::left, true, 0.4f);
        }

        // bezel screws
        for (auto p : { juce::Point<float> (bezel.getX() + 7.0f, bezel.getY() + 7.0f),
                        juce::Point<float> (bezel.getRight() - 7.0f, bezel.getY() + 7.0f),
                        juce::Point<float> (bezel.getX() + 7.0f, bezel.getBottom() - 7.0f),
                        juce::Point<float> (bezel.getRight() - 7.0f, bezel.getBottom() - 7.0f) })
            screw (g, p, 2.6f, 0.4f);
    }

    void jewel (juce::Graphics& g, juce::Point<float> p, float r, juce::Colour col, float glow)
    {
        if (glow > 0.01f)
        {
            juce::ColourGradient grad (col.withAlpha (0.5f * glow), p.x, p.y,
                                       col.withAlpha (0.0f), p.x + r * 3.2f, p.y, true);
            g.setGradientFill (grad);
            g.fillEllipse (p.x - r * 3.2f, p.y - r * 3.2f, r * 6.4f, r * 6.4f);
        }
        g.setColour (juce::Colour (0xff201c19));
        g.fillEllipse (p.x - r - 1.5f, p.y - r - 1.5f, (r + 1.5f) * 2.0f, (r + 1.5f) * 2.0f);
        g.setGradientFill (juce::ColourGradient (col.brighter (0.5f * glow + 0.15f), p.x - r * 0.4f, p.y - r * 0.5f,
                                                 col.darker (0.8f), p.x + r * 0.5f, p.y + r * 0.5f, true));
        g.fillEllipse (p.x - r, p.y - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.25f + 0.4f * glow));
        g.fillEllipse (p.x - r * 0.45f, p.y - r * 0.55f, r * 0.5f, r * 0.35f);
    }
}

//==============================================================================
juce::Font PanelRenderer::panelFont (float height, bool bold, float kerning)
{
    juce::Font f (juce::FontOptions (height).withStyle (bold ? "Bold" : "Regular"));
    f.setExtraKerningFactor (kerning);
    return f;
}

juce::Colour PanelRenderer::faceBase()   { return cFace; }
juce::Colour PanelRenderer::faceDark()   { return cFaceDeep; }
juce::Colour PanelRenderer::print()      { return cPrint; }
juce::Colour PanelRenderer::printMuted() { return cPrintMuted; }
juce::Colour PanelRenderer::brass()      { return cBrass; }
juce::Colour PanelRenderer::bakelite()   { return cBakelite; }

//==============================================================================
void PanelRenderer::renderBackground (juce::Graphics& g, const PanelLayout& l, float scale)
{
    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::scale (scale));

    // ---- chassis ---------------------------------------------------------
    g.setColour (juce::Colour (0xff0d0c0b));
    g.fillRect (l.chassis);

    brushedMetal (g, l.earLeft, juce::Colour (0xffb6bac0), juce::Colour (0xff73777d), true);
    brushedMetal (g, l.earRight, juce::Colour (0xffb6bac0), juce::Colour (0xff73777d), true);

    // ears bevels + slots
    for (auto ear : { l.earLeft, l.earRight })
    {
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawLine (ear.getX(), ear.getY(), ear.getX(), ear.getBottom(), 1.2f);
        g.setColour (juce::Colours::white.withAlpha (0.20f));
        g.drawLine (ear.getRight() - 1.0f, ear.getY(), ear.getRight() - 1.0f, ear.getBottom(), 1.0f);

        const bool left = ear.getX() < 1.0f;
        const float slotX = left ? ear.getRight() - 34.0f : ear.getX() + 12.0f;
        for (float y : { 128.0f, 472.0f })
        {
            juce::Rectangle<float> slot (slotX, y, 22.0f, 34.0f);
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.fillRoundedRectangle (slot.expanded (1.6f), 9.0f);
            g.setColour (juce::Colour (0xff2b2c2e));
            g.fillRoundedRectangle (slot, 8.0f);
            g.setColour (juce::Colours::white.withAlpha (0.16f));
            g.drawRoundedRectangle (slot, 8.0f, 1.0f);
        }
    }

    // top and bottom rails around the panel
    {
        auto top = juce::Rectangle<float> (l.earLeft.getRight(), 0.0f, l.earRight.getX() - l.earLeft.getRight(), 8.0f);
        auto bottom = top.translated (0.0f, 592.0f);
        for (auto rail : { top, bottom })
        {
            brushedMetal (g, rail, juce::Colour (0xff8f949a), juce::Colour (0xff4c5055), false);
            g.setColour (juce::Colours::white.withAlpha (0.25f));
            g.drawLine (rail.getX(), rail.getY() + 0.6f, rail.getRight(), rail.getY() + 0.6f, 0.8f);
        }
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.drawLine (l.face.getX(), l.face.getY(), l.face.getRight(), l.face.getY(), 2.0f);
    }

    // ---- face panel ------------------------------------------------------
    weatheredPanel (g, l.face);

    // printed labels
    {
        juce::Rectangle<float> r (l.xlrInputLabel.x - 70.0f, l.xlrInputLabel.y - 9.0f, 140.0f, 18.0f);
        text (g, "INPUT", r, 11.5f, cPrint, juce::Justification::centred, true, 0.3f);
        r = { l.xlrOutputLabel.x - 70.0f, l.xlrOutputLabel.y - 9.0f, 140.0f, 18.0f };
        text (g, "OUTPUT", r, 11.5f, cPrint, juce::Justification::centred, true, 0.3f);
    }

    for (auto x : l.xlrInput)  xlr (g, x);
    for (auto x : l.xlrOutput) xlr (g, x);

    // knobs: rings and printed labels
    for (int i = 0; i < PanelLayout::numKnobs; ++i)
    {
        if (auto* k = const_cast<PanelLayout&> (l).knobFor (i))
        {
            scaleRing (g, *k);
            if (i < 7)                     // the last two live in the control strip
                knobLabel (g, *k);
        }
    }

    meterFace (g, l);

    // brand plate under the meter
    {
        auto plate = l.brandPlate;
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.fillRoundedRectangle (plate.translated (0.0f, 1.0f), 4.0f);
        g.setColour (cBrass.withAlpha (0.12f));
        g.fillRoundedRectangle (plate, 4.0f);
        g.setColour (cBrass.withAlpha (0.5f));
        g.drawRoundedRectangle (plate, 4.0f, 1.0f);

        // emblem
        const auto c = juce::Point<float> (plate.getCentreX(), plate.getY() + 14.0f);
        juce::Path shield;
        shield.addTriangle (c.x - 5.0f, c.y - 5.0f, c.x + 5.0f, c.y - 5.0f, c.x, c.y + 6.0f);
        g.setColour (cBrass);
        g.fillPath (shield);

        text (g, "EMBER", { plate.getX(), plate.getY() + 18.0f, plate.getWidth(), 16.0f },
              13.0f, cPrint, juce::Justification::centred, true, 0.5f);
        text (g, "LIMITING AMPLIFIER  NO. 175-B", { plate.getX(), plate.getY() + 34.0f, plate.getWidth(), 12.0f },
              8.2f, cPrintMuted, juce::Justification::centred, true, 0.34f);
    }

    // power switch escutcheon + labels
    {
        auto p = l.power;
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (p, 6.0f);
        g.setColour (juce::Colour (0xff1b1714));
        g.fillRoundedRectangle (p, 5.0f);
        g.setColour (cPrintMuted.withAlpha (0.6f));
        g.drawRoundedRectangle (p, 5.0f, 1.0f);

        text (g, "ON", { p.getX() - 6.0f, p.getY() - 26.0f, p.getWidth() + 12.0f, 12.0f },
              8.5f, cPrint, juce::Justification::centred, true, 0.3f);
        text (g, "OFF", { p.getX() - 6.0f, p.getBottom() + 8.0f, p.getWidth() + 12.0f, 12.0f },
              8.5f, cPrintMuted.withAlpha (0.75f), juce::Justification::centred, true, 0.3f);
    }

    // ---- extended control strip ------------------------------------------
    {
        auto s = l.strip;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff26221f), s.getCentreX(), s.getY(),
                                                 juce::Colour (0xff14110f), s.getCentreX(), s.getBottom(), false));
        g.fillRoundedRectangle (s, 8.0f);
        g.setColour (juce::Colour (0xff6a5844).withAlpha (0.7f));
        g.drawRoundedRectangle (s, 8.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.drawRoundedRectangle (s.reduced (2.0f), 7.0f, 0.8f);

        juce::Random rng (0x9E17);
        for (int i = 0; i < 400; ++i)
        {
            g.setColour (juce::Colours::white.withAlpha (0.01f + rng.nextFloat() * 0.02f));
            g.fillRect (s.getX() + rng.nextFloat() * s.getWidth(), s.getY() + rng.nextFloat() * s.getHeight(), 1.0f, 1.0f);
        }

        text (g, "EXTENDED CONTROLS", { s.getX() + 14.0f, s.getY() + 6.0f, 250.0f, 11.0f },
              7.6f, cBrass.withAlpha (0.7f), juce::Justification::left, true, 0.32f);

        // labels for strip knobs are drawn as part of the knob spec, but the
        // strip runs below the face so they need the darker palette
        for (int i = 7; i < 9; ++i)
            if (auto* k = const_cast<PanelLayout&> (l).knobFor (i))
                text (g, k->label,
                      { k->centre.x - 70.0f, k->centre.y + k->radius + 5.0f, 140.0f, 11.0f },
                      7.8f, cPrintMuted, juce::Justification::centred, true, 0.2f);

        // toggle labels
        text (g, "STEREO LINK", { l.linkButton.getX(), l.linkButton.getY() - 2.0f, l.linkButton.getWidth(), 14.0f },
              8.2f, cPrintMuted, juce::Justification::centred, true, 0.18f);
        text (g, "STEPPED / CONTINUOUS", { l.vernierButton.getX(), l.vernierButton.getY() - 2.0f, l.vernierButton.getWidth(), 14.0f },
              8.2f, cPrintMuted, juce::Justification::centred, true, 0.12f);
        text (g, "METER", { l.meterButton.getX(), l.meterButton.getY() - 2.0f, l.meterButton.getWidth(), 14.0f },
              8.2f, cPrintMuted, juce::Justification::centred, true, 0.18f);

        text (g, "EMBER 175-B  ·  REFERENCE EMULATION  ·  NOT AFFILIATED WITH UNIVERSAL AUDIO",
              { s.getX() + 420.0f, s.getBottom() - 15.0f, s.getWidth() - 434.0f, 11.0f },
              7.2f, cPrintMuted.withAlpha (0.55f), juce::Justification::right, false, 0.1f);
    }

    // face screws
    for (auto p : l.screws)
        screw (g, p, 5.2f, 0.6f);
}

//==============================================================================
void PanelRenderer::drawKnobRing (juce::Graphics& g, const KnobSpec& k, float value)
{
    const auto c = k.centre;
    const float a = k.angleMin + juce::jlimit (0.0f, 1.0f, value) * (k.angleMax - k.angleMin);
    const auto dir = juce::Point<float> (std::sin (a), -std::cos (a));

    // small triangle marker just outside the printed ring
    const auto tip = c + dir * (k.ringRadius + (k.style == KnobStyle::large ? 30.0f : 21.0f));
    juce::Path p;
    p.addTriangle (tip.x, tip.y,
                   tip.x - dir.y * 3.4f - dir.x * 6.0f, tip.y + dir.x * 3.4f - dir.y * 6.0f,
                   tip.x + dir.y * 3.4f - dir.x * 6.0f, tip.y - dir.x * 3.4f - dir.y * 6.0f);
    g.setColour (cBrass.withAlpha (0.9f));
    g.fillPath (p);
}

void PanelRenderer::paintKnobComponent (juce::Graphics& g, const KnobSpec& k, juce::Rectangle<float> localArea,
                                        float normalised, bool highlighted, bool dragging)
{
    if (localArea.isEmpty())
        return;

    // design units -> component pixels: scale first, then translate so the
    // knob centre lands exactly on the component centre
    const float px = localArea.getHeight() / (k.radius * 2.6f);
    const auto centre = localArea.getCentre();

    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::scale (px)
                        .translated (centre.x - k.centre.x * px, centre.y - k.centre.y * px));

    drawKnob (g, k, normalised, highlighted, dragging);
}

void PanelRenderer::drawKnob (juce::Graphics& g, const KnobSpec& k, float normalised,
                              bool highlighted, bool dragging)
{
    const auto c = k.centre;
    const float r = k.radius;
    const float value = juce::jlimit (0.0f, 1.0f, normalised);
    const float angle = k.angleMin + value * (k.angleMax - k.angleMin);

    // drop shadow
    {
        juce::ColourGradient sh (juce::Colours::black.withAlpha (0.5f), c.x, c.y + r * 0.55f,
                                 juce::Colours::transparentBlack, c.x, c.y + r * 1.75f, true);
        g.setGradientFill (sh);
        g.fillEllipse (c.x - r * 1.25f, c.y - r * 0.35f, r * 2.5f, r * 2.4f);
    }

    // skirt / step
    if (k.style == KnobStyle::large)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a2f28), c.x - r * 0.4f, c.y - r * 0.5f,
                                                 juce::Colour (0xff0d0a08), c.x + r * 0.4f, c.y + r * 0.6f, true));
        g.fillEllipse (c.x - r * 1.12f, c.y - r * 1.12f, r * 2.24f, r * 2.24f);
        g.setColour (juce::Colour (0x66000000));
        g.drawEllipse (c.x - r * 1.12f, c.y - r * 1.12f, r * 2.24f, r * 2.24f, 1.0f);
    }

    // body
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a3c33), c.x - r * 0.42f, c.y - r * 0.55f,
                                             juce::Colour (0xff181310), c.x + r * 0.30f, c.y + r * 0.55f, true));
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);

    // fluted edge
    const int flutes = k.style == KnobStyle::large ? 76 : 48;
    for (int i = 0; i < flutes; ++i)
    {
        const float a = static_cast<float> (i) / static_cast<float> (flutes) * kPi * 2.0f;
        const auto d = juce::Point<float> (std::cos (a), std::sin (a));
        g.setColour (juce::Colours::black.withAlpha (i % 2 == 0 ? 0.30f : 0.12f));
        g.drawLine ({ c + d * (r * 0.90f), c + d * (r * 0.995f) }, 1.4f);
        g.setColour (juce::Colours::white.withAlpha (i % 2 == 0 ? 0.05f : 0.02f));
        g.drawLine ({ c + d * (r * 0.88f), c + d * (r * 0.90f) }, 1.0f);
    }

    // rim highlight and inner step
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawEllipse (c.x - r * 0.995f, c.y - r * 0.995f, r * 1.99f, r * 1.99f, 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawEllipse (c.x - r * 0.86f, c.y - r * 0.86f, r * 1.72f, r * 1.72f, 1.2f);

    // domed centre
    {
        juce::ColourGradient dome (juce::Colours::white.withAlpha (0.11f), c.x - r * 0.30f, c.y - r * 0.42f,
                                   juce::Colours::transparentBlack, c.x + r * 0.25f, c.y + r * 0.35f, true);
        g.setGradientFill (dome);
        g.fillEllipse (c.x - r * 0.84f, c.y - r * 0.84f, r * 1.68f, r * 1.68f);
    }

    // pointer
    {
        const auto dir = juce::Point<float> (std::sin (angle), -std::cos (angle));
        const auto nrm = juce::Point<float> (-dir.y, dir.x);
        juce::Path ptr;
        ptr.addTriangle (c.x + dir.x * r * 0.92f, c.y + dir.y * r * 0.92f,
                         c.x - nrm.x * 2.1f + dir.x * r * 0.34f, c.y - nrm.y * 2.1f + dir.y * r * 0.34f,
                         c.x + nrm.x * 2.1f + dir.x * r * 0.34f, c.y + nrm.y * 2.1f + dir.y * r * 0.34f);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillPath (ptr, juce::AffineTransform::translation (0.8f, 1.0f));
        g.setColour (juce::Colour (0xfff1e9d6).withAlpha (0.94f));
        g.fillPath (ptr);
        g.setColour (juce::Colour (0xfff1e9d6).withAlpha (0.55f));
        g.fillEllipse (c.x + dir.x * r * 0.94f - 1.6f, c.y + dir.y * r * 0.94f - 1.6f, 3.2f, 3.2f);
    }

    // interaction feedback, kept very subtle so it still looks like hardware
    if (highlighted || dragging)
    {
        g.setColour (cBrass.withAlpha (dragging ? 0.55f : 0.28f));
        g.drawEllipse (c.x - r * 1.02f, c.y - r * 1.02f, r * 2.04f, r * 2.04f, dragging ? 2.0f : 1.2f);
    }
}

//==============================================================================
void PanelRenderer::drawMeterNeedle (juce::Graphics& g, const PanelLayout& l, const MeterState& m)
{
    const auto face = l.meterFace;
    const float w = face.getWidth();
    const float h = face.getHeight();

    // scale geometry, matched by the printed face below
    const auto pivot = juce::Point<float> (face.getCentreX(), face.getY() + h * 0.94f);
    const float len = h * 0.86f;
    const float sweep = kPi * 0.30f;

    float norm;
    if (m.grMode)
        norm = juce::jlimit (0.0f, 1.0f, m.grDb / 20.0f);
    else
        norm = juce::jlimit (0.0f, 1.0f, (m.levelDb + 20.0f) / 23.0f);

    const float a = -sweep + norm * sweep * 2.0f;
    const auto dir = juce::Point<float> (std::sin (a), -std::cos (a));
    const auto tip = pivot + dir * len;

    // shadow then needle
    g.setColour (juce::Colours::black.withAlpha (0.20f));
    g.drawLine ({ pivot + juce::Point<float> (1.2f, 1.4f), tip + juce::Point<float> (1.2f, 1.4f) }, 2.0f);
    g.setColour (cNeedle);
    g.drawLine ({ pivot, tip }, 1.7f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2320), pivot.x - 7.0f, pivot.y - 7.0f,
                                             juce::Colour (0xff050505), pivot.x + 7.0f, pivot.y + 7.0f, true));
    g.fillEllipse (pivot.x - 6.5f, pivot.y - 6.5f, 13.0f, 13.0f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.fillEllipse (pivot.x - 3.0f, pivot.y - 3.6f, 2.6f, 2.0f);

    // lamp glow behind the scale when the unit is on
    if (m.powered)
    {
        juce::ColourGradient glow (juce::Colour (0x2bffd9a0), face.getCentreX(), face.getY() + h * 0.15f,
                                   juce::Colours::transparentBlack, face.getCentreX(), face.getBottom(), true);
        g.setGradientFill (glow);
        g.fillRoundedRectangle (face.reduced (2.0f), 4.0f);
    }
}

void PanelRenderer::drawMeterGlass (juce::Graphics& g, const PanelLayout& l, const MeterState&)
{
    const auto face = l.meterFace;

    // glass reflection
    juce::Path shine;
    shine.addQuadrilateral (face.getX(), face.getY(),
                            face.getX() + face.getWidth() * 0.55f, face.getY(),
                            face.getX() + face.getWidth() * 0.22f, face.getBottom(),
                            face.getX(), face.getBottom());
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.16f), face.getX(), face.getY(),
                                             juce::Colours::white.withAlpha (0.02f), face.getRight(), face.getBottom(), false));
    g.fillPath (shine);

    // subtle top tint
    g.setGradientFill (juce::ColourGradient (juce::Colour (0x22a8c4d8), face.getX(), face.getY(),
                                             juce::Colours::transparentBlack, face.getX(), face.getY() + face.getHeight() * 0.4f, false));
    g.fillRoundedRectangle (face.reduced (1.0f), 4.0f);

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawRoundedRectangle (face.reduced (0.5f), 4.0f, 1.0f);
}

//==============================================================================
void PanelRenderer::drawToggle (juce::Graphics& g, juce::Rectangle<float> area, bool on,
                                const juce::String& label, bool powerStyle)
{
    if (powerStyle)
    {
        g.setColour (juce::Colour (0xff13100e));
        g.fillRoundedRectangle (area, 5.0f);

        // bat handle
        const auto c = area.getCentre();
        const float h = area.getHeight() * (on ? 0.52f : 0.34f);
        juce::Rectangle<float> lever (c.x - 8.0f, on ? c.y - h : c.y - h * 0.2f + 6.0f, 16.0f, h);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe6e8ea), lever.getX(), lever.getY(),
                                                 juce::Colour (0xff6d7175), lever.getRight(), lever.getBottom(), false));
        g.fillRoundedRectangle (lever, 7.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.drawRoundedRectangle (lever, 7.0f, 0.9f);
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawLine (lever.getX() + 4.0f, lever.getY() + 3.0f, lever.getRight() - 4.0f, lever.getY() + 3.0f, 1.0f);

        jewel (g, { area.getCentreX(), area.getBottom() + 2.0f }, 4.6f, juce::Colour (0xffff8a3c), on ? 1.0f : 0.0f);
        return;
    }

    // mini toggle: brass LED + label
    const auto led = juce::Point<float> (area.getX() + 12.0f, area.getCentreY());
    jewel (g, led, 4.4f, on ? juce::Colour (0xffff7a32) : juce::Colour (0xff6b4a30), on ? 1.0f : 0.0f);

    juce::Rectangle<float> textArea (area.getX() + 24.0f, area.getY(), area.getWidth() - 24.0f, area.getHeight());
    text (g, label, textArea, 9.0f, on ? cPrint : cPrintMuted.withAlpha (0.7f), juce::Justification::centredLeft, true, 0.16f);
}

void PanelRenderer::drawReadout (juce::Graphics& g, juce::Rectangle<float> area,
                                  const juce::String& big, const juce::String& small, float activity)
{
    g.setColour (juce::Colour (0xff0e1210));
    g.fillRoundedRectangle (area, 4.0f);
    g.setColour (juce::Colour (0xff2a2a26));
    g.drawRoundedRectangle (area, 4.0f, 1.0f);

    // faint scanlines
    for (float y = area.getY() + 1.0f; y < area.getBottom(); y += 3.0f)
    {
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.fillRect (area.getX() + 1.0f, y, area.getWidth() - 2.0f, 1.0f);
    }

    juce::Font mono (juce::FontOptions (20.0f).withStyle ("Bold"));
    g.setColour (juce::Colour (0xffe0b070).withAlpha (0.75f + 0.25f * activity));
    g.setFont (mono);
    g.drawText (big, area.reduced (10.0f, 6.0f).withTrimmedBottom (area.getHeight() * 0.42f),
                juce::Justification::centredLeft, false);

    g.setColour (juce::Colour (0xff8fa79a).withAlpha (0.8f));
    g.setFont (PanelRenderer::panelFont (8.5f, false, 0.14f));
    g.drawText (small, area.reduced (10.0f, 6.0f).withTrimmedTop (area.getHeight() * 0.5f),
                juce::Justification::centredLeft, false);
}
}
