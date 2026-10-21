#include "PanelRenderer.h"

namespace ember::ui
{
namespace
{
    constexpr float kPi = juce::MathConstants<float>::pi;

    // ------------------------------------------------------------------ palette
    // sampled from the reference panel: oxblood face, cream silkscreen, copper
    // trim, brushed aluminium knobs, amber meter card.
    const juce::Colour cCaseTop    { 0xff2b2c2e };
    const juce::Colour cCaseBot    { 0xff0b0b0c };
    const juce::Colour cFaceTop    { 0xff33181b };
    const juce::Colour cFaceMid    { 0xff2d1416 };
    const juce::Colour cFaceBot    { 0xff170809 };
    const juce::Colour cPrint      { 0xffe9dcc8 };
    const juce::Colour cPrintDim   { 0xffb39c88 };
    const juce::Colour cCopper     { 0xffc08a63 };
    const juce::Colour cTrim       { 0xffe6c3b6 };
    const juce::Colour cAluHi      { 0xfff6f7f8 };
    const juce::Colour cAluMid     { 0xff9aa0a6 };
    const juce::Colour cAluLo     { 0xff43474c };
    const juce::Colour cSkirtHi    { 0xff2a2529 };
    const juce::Colour cSkirtLo    { 0xff070608 };
    const juce::Colour cInk        { 0xff2b1c10 };
    const juce::Colour cRed        { 0xffa8201a };
    const juce::Colour cCardTop    { 0xfff9c584 };
    const juce::Colour cCardMid    { 0xffe8a95f };
    const juce::Colour cCardBot    { 0xffbf7b36 };
    const juce::Colour cBronzeHi   { 0xff6d5a49 };
    const juce::Colour cBronzeLo   { 0xff1a1310 };

    void text (juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r,
               float height, juce::Colour colour,
               juce::Justification just = juce::Justification::centred,
               bool bold = true, float kerning = 0.2f)
    {
        if (s.isEmpty())
            return;
        g.setColour (colour);
        g.setFont (PanelRenderer::panelFont (height, bold, kerning));
        g.drawText (s, r, just, false);
    }

    /** Text centred on a point, with a box wide enough for a long caption. */
    void centredText (juce::Graphics& g, const juce::String& s, juce::Point<float> p,
                      float height, juce::Colour colour, float kerning = 0.25f, float boxWidth = 220.0f)
    {
        text (g, s, { p.x - boxWidth * 0.5f, p.y - height * 0.75f, boxWidth, height * 1.5f },
              height, colour, juce::Justification::centred, true, kerning);
    }

    void hairline (juce::Graphics& g, float x1, float x2, float y)
    {
        juce::ColourGradient line (juce::Colours::transparentBlack, x1, y,
                                   cCopper.withAlpha (0.85f), (x1 + x2) * 0.5f, y, false);
        line.addColour (1.0, juce::Colours::transparentBlack);
        g.setGradientFill (line);
        g.fillRect (x1, y, x2 - x1, 1.1f);
        g.setColour (juce::Colours::black.withAlpha (0.30f));
        g.fillRect (x1, y + 1.1f, x2 - x1, 0.9f);
    }

    /** Slotted chrome screw. */
    void screw (juce::Graphics& g, juce::Point<float> c, float r, float angle)
    {
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillEllipse (c.x - r * 1.08f, c.y - r * 1.08f, r * 2.16f, r * 2.16f);
        g.setGradientFill (juce::ColourGradient (cAluHi, c.x - r * 0.6f, c.y - r * 0.7f,
                                                 cAluLo, c.x + r * 0.6f, c.y + r * 0.7f, true));
        g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawEllipse (c.x - r * 0.92f, c.y - r * 0.92f, r * 1.84f, r * 1.84f, 0.5f);

        g.saveState();
        juce::Path slot;
        slot.addRectangle (-r * 0.78f, -r * 0.16f, r * 1.56f, r * 0.32f);
        g.addTransform (juce::AffineTransform::rotation (angle).translated (c.x, c.y));
        g.setColour (juce::Colours::black.withAlpha (0.65f));
        g.fillPath (slot);
        g.restoreState();
    }

    // ------------------------------------------------------------------ face
    /** Brushed aluminium rack ear with its oval mounting slots. */
    void paintEar (juce::Graphics& g, juce::Rectangle<float> ear, const juce::Rectangle<float> slots[2], int seed)
    {
        if (ear.isEmpty())
            return;

        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (ear.expanded (1.0f).translated (0.0f, 1.5f), 5.0f);

        juce::ColourGradient alu (juce::Colour (0xff9fa1a3), ear.getX(), ear.getY(),
                                  juce::Colour (0xff3f4144), ear.getX() + ear.getWidth(), ear.getY(), false);
        alu.addColour (0.3, juce::Colour (0xff8b8d90));
        alu.addColour (0.7, juce::Colour (0xff63666a));
        g.setGradientFill (alu);
        g.fillRoundedRectangle (ear, 5.0f);

        juce::Random rng (seed);
        for (int i = 0; i < 160; ++i)
        {
            const float x = ear.getX() + rng.nextFloat() * ear.getWidth();
            g.setColour ((rng.nextBool() ? juce::Colours::white : juce::Colours::black)
                             .withAlpha (0.03f + rng.nextFloat() * 0.06f));
            g.fillRect (x, ear.getY() + 1.5f, 0.8f, ear.getHeight() - 3.0f);
        }

        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.drawRoundedRectangle (ear, 5.0f, 0.9f);

        for (int i = 0; i < 2; ++i)
        {
            const auto slot = slots[i];
            if (slot.isEmpty())
                continue;
            const float rr = slot.getHeight() * 0.5f;
            g.setColour (juce::Colour (0xff131211));
            g.fillRoundedRectangle (slot, rr);
            g.setColour (juce::Colours::white.withAlpha (0.30f));
            g.drawRoundedRectangle (slot.translated (0.0f, 1.2f), rr, 0.9f);
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.drawRoundedRectangle (slot, rr, 0.8f);
        }
    }

    void paintCase (juce::Graphics& g, const PanelLayout& l)
    {
        g.setColour (juce::Colour (0xff0a0909));
        g.fillRect (l.panel.expanded (4.0f));

        // rack ears either side
        paintEar (g, l.ears[0], l.earSlots, 0x3a11);
        paintEar (g, l.ears[1], l.earSlots + 2, 0x7c22);

        g.setGradientFill (juce::ColourGradient (cCaseTop, l.panelCase.getCentreX(), l.panelCase.getY(),
                                                 cCaseBot, l.panelCase.getCentreX(), l.panelCase.getBottom(), false));
        g.fillRoundedRectangle (l.panelCase, l.cornerRadius);

        // wine face, lit from the top
        juce::ColourGradient face (cFaceTop, l.face.getCentreX(), l.face.getY(),
                                   cFaceBot, l.face.getCentreX(), l.face.getBottom(), false);
        face.addColour (0.42, cFaceMid);
        g.setGradientFill (face);
        g.fillRoundedRectangle (l.face, l.cornerRadius * 0.55f);

        // warm key light: a pool behind the meter, falling off to the edges
        {
            juce::ColourGradient glow (juce::Colour (0x3cd18a4a), l.face.getCentreX(), l.face.getY() + 30.0f,
                                       juce::Colours::transparentBlack, l.face.getCentreX(), l.face.getBottom(), true);
            glow.addColour (0.0, juce::Colour (0x2ec07040));
            glow.addColour (0.55, juce::Colour (0x0cb0561f));
            g.setGradientFill (glow);
            g.fillRoundedRectangle (l.face, l.cornerRadius * 0.55f);
        }

        // vignette
        {
            juce::ColourGradient vig (juce::Colours::transparentBlack, l.face.getCentreX(), l.face.getCentreY(),
                                      juce::Colours::black.withAlpha (0.45f), l.face.getX(), l.face.getY(), true);
            vig.addColour (0.55, juce::Colours::black.withAlpha (0.05f));
            g.setGradientFill (vig);
            g.fillRoundedRectangle (l.face, l.cornerRadius * 0.55f);
        }

        // photographic grain, and the faint horizontal polish of the lacquer
        juce::Random rng (0x1753c3);
        for (int i = 0; i < 900; ++i)
        {
            const float x = l.face.getX() + rng.nextFloat() * l.face.getWidth();
            const float y = l.face.getY() + rng.nextFloat() * l.face.getHeight();
            g.setColour (rng.nextBool() ? juce::Colours::white.withAlpha (0.010f + rng.nextFloat() * 0.022f)
                                        : juce::Colours::black.withAlpha (0.02f + rng.nextFloat() * 0.05f));
            g.fillRect (x, y, 1.0f + rng.nextFloat() * 2.5f, 1.0f + rng.nextFloat());
        }
        for (int i = 0; i < 70; ++i)
        {
            const float y = l.face.getY() + rng.nextFloat() * l.face.getHeight();
            g.setColour (juce::Colours::white.withAlpha (0.006f + rng.nextFloat() * 0.012f));
            g.fillRect (l.face.getX(), y, l.face.getWidth(), 0.7f + rng.nextFloat());
        }

        // bright case lip along the top, copper trim just inside it
        {
            juce::ColourGradient lip (juce::Colours::transparentBlack, l.face.getX(), l.face.getY(),
                                      juce::Colours::transparentBlack, l.face.getRight(), l.face.getY(), false);
            lip.addColour (0.18, cTrim.withAlpha (0.85f));
            lip.addColour (0.5, juce::Colours::white.withAlpha (0.9f));
            lip.addColour (0.82, cTrim.withAlpha (0.8f));
            g.setGradientFill (lip);
            g.drawLine (l.face.getX() + 6.0f, l.face.getY() + 0.6f,
                        l.face.getRight() - 6.0f, l.face.getY() + 0.6f, 1.2f);

            g.setColour (cCopper.withAlpha (0.32f));
            g.drawRoundedRectangle (l.panelCase.reduced (3.2f), l.cornerRadius * 0.8f, 0.8f);
            g.setColour (juce::Colours::black.withAlpha (0.5f));
            g.drawRoundedRectangle (l.panelCase.reduced (5.0f), l.cornerRadius * 0.7f, 0.9f);
        }

        for (auto& s : l.screws)
            screw (g, s, l.screwRadius, 0.6f);
    }

    // ------------------------------------------------------------------ meter
    // Positions where a moving coil VU meter prints its marks: -20 .. -5 is
    // compressed, the last few dB around zero are opened up.
    struct VUPoint { int db; float pos; };
    static const VUPoint vuScale[] = {
        { -20, 0.000f }, { -15, 0.130f }, { -10, 0.262f }, { -7, 0.398f },
        {  -5, 0.502f }, {  -3, 0.600f }, {  -2, 0.660f }, { -1, 0.730f },
        {   0, 0.800f }, {   1, 0.868f }, {   2, 0.932f }, {  3, 1.000f }
    };

    static float posForDb (int db)
    {
        constexpr int n = static_cast<int> (sizeof (vuScale) / sizeof (vuScale[0]));
        if (db <= vuScale[0].db)      return vuScale[0].pos;
        if (db >= vuScale[n - 1].db)  return vuScale[n - 1].pos;
        for (int i = 1; i < n; ++i)
            if (db <= vuScale[i].db)
            {
                const float t = static_cast<float> (db - vuScale[i - 1].db)
                              / static_cast<float> (vuScale[i].db - vuScale[i - 1].db);
                return vuScale[i - 1].pos + t * (vuScale[i].pos - vuScale[i - 1].pos);
            }
        return 1.0f;
    }

    static float posForDb (float db)
    {
        const int lo = static_cast<int> (std::floor (db));
        const int hi = static_cast<int> (std::ceil (db));
        if (lo == hi) return posForDb (lo);
        return posForDb (lo) + (db - static_cast<float> (lo)) * (posForDb (hi) - posForDb (lo));
    }

    struct MeterGeometry
    {
        juce::Point<float> pivot;
        float radius = 60.0f;
        juce::Rectangle<float> card;
    };

    MeterGeometry meterGeometry (const PanelLayout& l)
    {
        MeterGeometry m;
        m.card = l.meterCard;
        m.radius = m.card.getHeight() * 0.71f;
        m.pivot = { m.card.getCentreX(), m.card.getBottom() - 9.0f };
        return m;
    }

    constexpr float kSweep = 1.05f;     // radians either side of vertical

    float angleFor (float norm)
    {
        return -kSweep + juce::jlimit (0.0f, 1.0f, norm) * 2.0f * kSweep;
    }

    void paintMeter (juce::Graphics& g, const PanelLayout& l)
    {
        const auto bezel = l.meter;
        const auto g2 = meterGeometry (l);
        const auto card = g2.card;

        // cast shadow
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (bezel.translated (0.0f, 2.5f).expanded (1.5f), 6.0f);

        // bronze bezel
        g.setGradientFill (juce::ColourGradient (cBronzeHi, bezel.getX(), bezel.getY(),
                                                 cBronzeLo, bezel.getRight(), bezel.getBottom(), false));
        g.fillRoundedRectangle (bezel, 4.5f);
        g.setColour (juce::Colours::black.withAlpha (0.75f));
        g.drawRoundedRectangle (bezel, 4.5f, 1.2f);
        {
            juce::ColourGradient lip (juce::Colours::transparentBlack, bezel.getX(), bezel.getY(),
                                      juce::Colours::transparentBlack, bezel.getRight(), bezel.getY(), false);
            lip.addColour (0.2, juce::Colours::white.withAlpha (0.55f));
            lip.addColour (0.55, juce::Colours::white.withAlpha (0.28f));
            lip.addColour (0.9, juce::Colours::transparentBlack);
            g.setGradientFill (lip);
            g.drawLine (bezel.getX() + 4.0f, bezel.getY() + 0.8f, bezel.getRight() - 4.0f, bezel.getY() + 0.8f, 1.0f);
        }

        // card well
        g.setColour (juce::Colours::black.withAlpha (0.85f));
        g.fillRoundedRectangle (card.expanded (2.5f), 2.5f);

        // amber card
        juce::ColourGradient amber (cCardTop, card.getCentreX(), card.getY(),
                                    juce::Colour (0xffa86526), card.getCentreX(), card.getBottom(), false);
        amber.addColour (0.35, cCardMid);
        amber.addColour (0.75, juce::Colour (0xffcf8c42));
        g.setGradientFill (amber);
        g.fillRect (card);

        // lamp behind the card, glowing from the bottom
        {
            juce::ColourGradient glow (juce::Colour (0x9cf6a94a), card.getCentreX(), card.getBottom(),
                                       juce::Colours::transparentBlack, card.getCentreX(), card.getY(), true);
            g.setGradientFill (glow);
            g.fillRect (card);
        }

        // paper grain and the dark corners of an old card
        juce::Random rng (0x00c175);
        for (int i = 0; i < 320; ++i)
        {
            g.setColour (juce::Colour (0xff8a5a1e).withAlpha (0.02f + rng.nextFloat() * 0.05f));
            g.fillEllipse (card.getX() + rng.nextFloat() * card.getWidth(),
                           card.getY() + rng.nextFloat() * card.getHeight(), 1.6f, 1.6f);
        }
        {
            juce::ColourGradient vig (juce::Colours::transparentBlack, card.getCentreX(), card.getCentreY(),
                                      juce::Colour (0x66000000), card.getX(), card.getY(), true);
            vig.addColour (0.6, juce::Colours::transparentBlack);
            g.setGradientFill (vig);
            g.fillRect (card);
        }

        // ---- printed scale
        const auto pivot = g2.pivot;
        const float r = g2.radius;

        juce::Path arc;
        arc.addCentredArc (pivot.x, pivot.y, r, r, 0.0f, angleFor (0.0f), angleFor (1.0f), true);
        g.setColour (cInk.withAlpha (0.92f));
        g.strokePath (arc, juce::PathStrokeType (1.4f));

        // a tick for every dB on the scale, a printed number where the card has one
        auto isLabelled = [] (int db)
        {
            return db == -20 || db == -10 || db == -7 || db == -5 || db == -3
                || db == -1 || db == 0;
        };

        for (int db = -20; db <= 2; ++db)
        {
            const float a = angleFor (posForDb (db));
            const auto d = juce::Point<float> (std::sin (a), -std::cos (a));
            const bool label = isLabelled (db);
            const bool half = (db > -10) && (db % 2 != 0);

            g.setColour (db >= 0 ? cRed : cInk.withAlpha (label ? 0.95f : 0.75f));
            g.drawLine ({ pivot + d * (r - (label ? 5.5f : half ? 3.6f : 4.6f)), pivot + d * (r + 1.5f) },
                        label ? 1.6f : (half ? 0.9f : 1.1f));

            if (! label)
                continue;

            // numbers sit just outside the arc and turn with it
            const auto p = pivot + d * (r + 11.0f);
            juce::Graphics::ScopedSaveState num (g);
            g.addTransform (juce::AffineTransform::rotation (a, p.x, p.y));
            g.setFont (PanelRenderer::panelFont (8.8f, false, 0.0f));
            g.setColour (cInk);
            g.drawText (juce::String (db), juce::Rectangle<float> (p.x - 13.0f, p.y - 6.0f, 26.0f, 12.0f),
                        juce::Justification::centred, false);
        }

        // red zone 0 .. +3, a band printed outside the numbers
        {
            juce::Path band;
            const float rr = r + 20.0f;
            band.addCentredArc (pivot.x, pivot.y, rr, rr, 0.0f, angleFor (posForDb (0)), angleFor (posForDb (3)), true);
            g.setColour (cRed);
            g.strokePath (band, juce::PathStrokeType (5.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            const float a = angleFor (posForDb (3));
            const auto d = juce::Point<float> (std::sin (a), -std::cos (a));
            const auto p = pivot + d * (rr + 14.0f);
            juce::Graphics::ScopedSaveState num (g);
            g.addTransform (juce::AffineTransform::rotation (a * 0.6f, p.x, p.y));
            g.setFont (PanelRenderer::panelFont (8.8f, false, 0.0f));
            g.setColour (cRed);
            g.drawText ("+3", juce::Rectangle<float> (p.x - 14.0f, p.y - 6.0f, 28.0f, 12.0f),
                        juce::Justification::centred, false);
        }

        // "VU" inside the arc, "dB" in the bottom right corner
        text (g, "VU", { pivot.x - 44.0f, pivot.y - 36.0f, 44.0f, 20.0f }, 15.0f, cInk.withAlpha (0.9f),
              juce::Justification::centred, true, 0.16f);
        text (g, "dB", { card.getRight() - 30.0f, pivot.y - 28.0f, 26.0f, 14.0f }, 10.0f, cInk.withAlpha (0.9f),
              juce::Justification::centred, true, 0.04f);

        // bezel screws
        for (auto& s : l.meterScrews)
            screw (g, s, l.meterScrewRadius, 0.9f);
    }

    // ------------------------------------------------------------------ brand
    void paintBrand (juce::Graphics& g, const PanelLayout& l)
    {
        const auto c = l.brandMark;
        const float hw = l.brandMarkHalfW, hh = l.brandMarkHalfH;

        juce::Path diamond;
        diamond.addQuadrilateral (c.x, c.y - hh, c.x + hw, c.y, c.x, c.y + hh, c.x - hw, c.y);
        g.setColour (cCopper.withAlpha (0.95f));
        g.strokePath (diamond, juce::PathStrokeType (1.5f));

        juce::Path inner;
        inner.addQuadrilateral (c.x, c.y - hh + 3.6f, c.x + hw - 5.2f, c.y,
                                c.x, c.y + hh - 3.6f, c.x - hw + 5.2f, c.y);
        g.setColour (cCopper.withAlpha (0.5f));
        g.strokePath (inner, juce::PathStrokeType (0.7f));

        const auto mark = l.brandMarkText.replace (".", juce::String::charToString ((juce::juce_wchar) 0xb7));
        text (g, mark, { c.x - hw, c.y - hh, hw * 2.0f, hh * 2.0f }, 21.0f, cCopper,
              juce::Justification::centred, false, 0.06f);

        text (g, "EMBER 175", l.brandName, 19.0f, cPrint, juce::Justification::centred, false, 0.42f);
        text (g, "TUBE COMPRESSOR", l.brandSub, 9.5f, cPrintDim, juce::Justification::centred, false, 0.62f);

        hairline (g, l.hairlineLeft[0], l.hairlineLeft[1], l.hairlineY);
        hairline (g, l.hairlineRight[0], l.hairlineRight[1], l.hairlineY);
    }

    // ------------------------------------------------------------------ print
    void paintTickRing (juce::Graphics& g, const KnobSpec& k)
    {
        const bool big = k.style == KnobStyle::big;

        for (int t = 0; t < k.tickCount; ++t)
        {
            const float n = k.tickCount > 1 ? static_cast<float> (t) / static_cast<float> (k.tickCount - 1) : 0.0f;
            const float a = k.angleMin + n * (k.angleMax - k.angleMin);
            const auto d = juce::Point<float> (std::sin (a), -std::cos (a));
            const bool major = big ? (t % 2 == 0) : (k.tickCount <= 9 || t % 2 == 0 || t == k.tickCount - 1);

            g.setColour (cPrint.withAlpha (major ? 0.90f : 0.50f));
            const float len = major ? (big ? 7.0f : 5.0f) : (big ? 4.4f : 3.2f);
            g.drawLine ({ k.centre + d * (k.tickRadius - len), k.centre + d * k.tickRadius },
                        major ? (big ? 1.4f : 1.15f) : 0.85f);
        }

        for (auto& m : k.marks)
        {
            const float a = k.angleMin + juce::jlimit (0.0f, 1.0f, m.position) * (k.angleMax - k.angleMin);
            const auto p = k.centre + juce::Point<float> (std::sin (a), -std::cos (a)) * k.labelRadius;
            const float h = k.labelHeight;
            g.setColour (cPrint.withAlpha (0.96f));
            g.setFont (PanelRenderer::panelFont (h, false, 0.04f));
            g.drawText (m.text,
                        juce::Rectangle<float> (p.x - 17.0f, p.y - h * 0.58f, 34.0f, h * 1.16f),
                        juce::Justification::centred, false);
        }
    }

    void paintKnobPrint (juce::Graphics& g, const KnobSpec& k)
    {
        paintTickRing (g, k);

        if (k.caption.isNotEmpty())
            centredText (g, k.caption, k.captionPos, k.style == KnobStyle::big ? 12.0f : 11.0f,
                         cPrint, 0.30f, 260.0f);

        if (k.leftLabel.isNotEmpty())
            text (g, k.leftLabel,
                  { k.leftLabelPos.x - 40.0f, k.leftLabelPos.y - 8.0f, 80.0f, 16.0f },
                  12.0f, cPrint, juce::Justification::centred, true, 0.10f);
        if (k.rightLabel.isNotEmpty())
            text (g, k.rightLabel,
                  { k.rightLabelPos.x - 40.0f, k.rightLabelPos.y - 8.0f, 80.0f, 16.0f },
                  12.0f, cPrint, juce::Justification::centred, true, 0.10f);

        if (k.offLabel.isNotEmpty())
            text (g, k.offLabel,
                  { k.offLabelPos.x - 40.0f, k.offLabelPos.y - 7.0f, 80.0f, 14.0f },
                  11.0f, cPrintDim, juce::Justification::centred, true, 0.14f);
    }
}

//==============================================================================
juce::Font PanelRenderer::panelFont (float height, bool bold, float kerning)
{
    juce::Font f (juce::FontOptions (height).withStyle (bold ? "Bold" : "Regular"));
    f.setExtraKerningFactor (kerning);
    return f;
}

juce::Colour PanelRenderer::faceDark()   { return cFaceBot; }
juce::Colour PanelRenderer::print()      { return cPrint; }
juce::Colour PanelRenderer::printMuted() { return cPrintDim; }
juce::Colour PanelRenderer::brass()      { return cCopper; }

//==============================================================================
void PanelRenderer::renderBackground (juce::Graphics& g, const PanelLayout& l, float scale)
{
    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::scale (scale));

    paintCase (g, l);
    paintMeter (g, l);
    paintBrand (g, l);

    for (int i = 0; i < PanelLayout::numKnobs; ++i)
        if (auto* k = l.knobFor (i))
            paintKnobPrint (g, *k);

    // S.C. LINK group
    centredText (g, "S.C. LINK", l.linkCaptionPos, 12.5f, cPrint, 0.26f, 220.0f);
    centredText (g, "OFF", l.linkOffLabelPos, 11.0f, cPrintDim, 0.16f, 120.0f);
    centredText (g, "ON", l.linkOnLabelPos, 11.0f, cPrintDim, 0.16f, 120.0f);

    // POWER group
    centredText (g, "POWER", l.powerCaptionPos, 12.0f, cPrint, 0.26f, 160.0f);
    centredText (g, "ON", l.powerOnLabelPos, 11.0f, cPrintDim, 0.16f, 120.0f);
}

//==============================================================================
void PanelRenderer::drawKnobScale (juce::Graphics& g, const KnobSpec& k)
{
    paintTickRing (g, k);
}

//==============================================================================
void PanelRenderer::paintKnobComponent (juce::Graphics& g, const KnobSpec& k, juce::Rectangle<float> localArea,
                                        float normalised, bool highlighted, bool dragging)
{
    if (localArea.isEmpty())
        return;

    const float px = localArea.getHeight() / (k.skirtRadius * 2.85f);
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
    const float sk = k.skirtRadius;
    const float cp = k.capRadius;
    const bool big = k.style == KnobStyle::big;
    const float value = juce::jlimit (0.0f, 1.0f, normalised);
    const float angle = k.angleMin + value * (k.angleMax - k.angleMin);

    // cast shadow, kept inside the component so it never clips
    {
        juce::ColourGradient sh (juce::Colours::black.withAlpha (0.60f), c.x, c.y + sk * 0.65f,
                                 juce::Colours::transparentBlack, c.x, c.y + sk * 1.38f, true);
        g.setGradientFill (sh);
        g.fillEllipse (c.x - sk * 1.22f, c.y - sk * 0.5f, sk * 2.44f, sk * 2.86f);
    }

    // ---- black bakelite skirt, lit from the top left, with a knurled edge
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff453d41), c.x - sk * 0.35f, c.y - sk * 0.55f,
                                             juce::Colour (0xff050506), c.x + sk * 0.45f, c.y + sk * 0.6f, true));
    g.fillEllipse (c.x - sk, c.y - sk, sk * 2.0f, sk * 2.0f);

    {
        const int flutes = big ? 96 : 64;
        for (int i = 0; i < flutes; ++i)
        {
            const float a = static_cast<float> (i) / static_cast<float> (flutes) * 2.0f * kPi;
            const auto d = juce::Point<float> (std::cos (a), std::sin (a));
            const float light = juce::jlimit (0.0f, 1.0f, 0.5f + 0.5f * std::cos (a + 2.3f));
            g.setColour (juce::Colours::white.withAlpha (0.05f + 0.20f * light * light));
            g.drawLine ({ c + d * (sk * 0.86f), c + d * (sk - 0.6f) }, sk * 0.055f);
        }
    }

    g.setColour (juce::Colours::black.withAlpha (0.9f));
    g.drawEllipse (c.x - sk, c.y - sk, sk * 2.0f, sk * 2.0f, 0.9f);

    // chrome ring where the skirt meets the cap
    {
        juce::Path ring;
        ring.addCentredArc (c.x, c.y, cp + 1.9f, cp + 1.9f, 0.0f, -2.55f, 1.0f, true);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.strokePath (ring, juce::PathStrokeType (1.1f));
    }

    // ---- brushed gunmetal cap
    juce::ColourGradient capGrad (juce::Colour (0xffb3aca3), c.x - cp * 0.42f, c.y - cp * 0.60f,
                                  juce::Colour (0xff17140f), c.x + cp * 0.55f, c.y + cp * 0.62f, true);
    capGrad.addColour (0.28, juce::Colour (0xff7d756b));
    capGrad.addColour (0.55, juce::Colour (0xff4b453d));
    capGrad.addColour (0.80, juce::Colour (0xff2c2823));
    capGrad.addColour (1.0, juce::Colour (0xff0d0b09));
    g.setGradientFill (capGrad);
    g.fillEllipse (c.x - cp, c.y - cp, cp * 2.0f, cp * 2.0f);

    {
        juce::Graphics::ScopedSaveState capState (g);
        juce::Path clip;
        clip.addEllipse (c.x - cp, c.y - cp, cp * 2.0f, cp * 2.0f);
        g.reduceClipRegion (clip, juce::AffineTransform());

        // radial sunburst turning marks, brighter towards the top left
        const int rays = big ? 220 : 150;
        for (int i = 0; i < rays; ++i)
        {
            const float a = static_cast<float> (i) / static_cast<float> (rays) * 2.0f * kPi;
            const auto d = juce::Point<float> (std::cos (a), std::sin (a));
            const float light = juce::jlimit (0.0f, 1.0f, 0.5f + 0.5f * std::cos (a + 2.3f));
            if ((i % 2) == 0)
                g.setColour (juce::Colour (0xfff5ece0).withAlpha (0.015f + 0.20f * light * light * light));
            else
                g.setColour (juce::Colours::black.withAlpha (0.06f + 0.22f * (1.0f - light)));
            g.drawLine ({ c + d * (cp * 0.12f), c + d * cp }, big ? 0.7f : 0.55f);
        }

        // concentric turning, very faint
        for (int i = 1; i <= 12; ++i)
        {
            const float rr = cp * (0.2f + 0.8f * static_cast<float> (i) / 12.0f);
            g.setColour ((i % 2 == 0 ? juce::Colours::white : juce::Colours::black).withAlpha (0.03f));
            g.drawEllipse (c.x - rr, c.y - rr, rr * 2.0f, rr * 2.0f, 0.7f);
        }

        // specular wedge on the upper left, deep shade lower right
        {
            juce::ColourGradient spec (juce::Colours::white.withAlpha (0.40f), c.x - cp * 0.5f, c.y - cp * 0.55f,
                                       juce::Colours::transparentBlack, c.x + cp * 0.05f, c.y + cp * 0.15f, false);
            g.setGradientFill (spec);
            g.fillEllipse (c.x - cp, c.y - cp, cp * 2.0f, cp * 2.0f);
        }
        {
            juce::ColourGradient shade (juce::Colours::transparentBlack, c.x, c.y - cp * 0.2f,
                                        juce::Colours::black.withAlpha (0.30f), c.x, c.y + cp, false);
            g.setGradientFill (shade);
            g.fillEllipse (c.x - cp, c.y - cp, cp * 2.0f, cp * 2.0f);
        }
    }

    // cap edge: chrome lip, bright where the light hits, dark on the far side
    {
        juce::Path lip;
        lip.addCentredArc (c.x, c.y, cp, cp, 0.0f, -2.45f, -0.55f, true);
        g.setColour (juce::Colour (0xfffff6ea).withAlpha (0.85f));
        g.strokePath (lip, juce::PathStrokeType (big ? 1.5f : 1.2f));

        juce::Path lipLo;
        lipLo.addCentredArc (c.x, c.y, cp, cp, 0.0f, 0.6f, 2.5f, true);
        g.setColour (juce::Colours::black.withAlpha (0.75f));
        g.strokePath (lipLo, juce::PathStrokeType (1.0f));

        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.drawEllipse (c.x - cp, c.y - cp, cp * 2.0f, cp * 2.0f, 0.7f);
    }

    // ---- indicator line
    {
        const auto d = juce::Point<float> (std::sin (angle), -std::cos (angle));
        const float w = big ? 2.2f : 1.6f;
        const auto from = c + d * (cp * 0.03f);
        const auto to = c + d * (cp * 0.97f);

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawLine ({ from + juce::Point<float> (0.7f, 0.9f), to + juce::Point<float> (0.7f, 0.9f) }, w + 1.4f);
        g.setColour (juce::Colour (0xffded9cd).withAlpha (0.55f));
        g.drawLine ({ from, to }, w + 2.2f);
        g.setColour (juce::Colour (0xfffffbf2));
        g.drawLine ({ from, to }, w);
    }

    if (highlighted || dragging)
    {
        g.setColour (juce::Colour (0xfff0d2a8).withAlpha (dragging ? 0.5f : 0.22f));
        g.drawEllipse (c.x - sk - 1.5f, c.y - sk - 1.5f, (sk + 1.5f) * 2.0f, (sk + 1.5f) * 2.0f,
                       dragging ? 1.5f : 0.9f);
    }
}

//==============================================================================
void PanelRenderer::drawToggle (juce::Graphics& g, juce::Rectangle<float> area, bool on)
{
    const auto c = area.getCentre();
    const float r = juce::jmin (area.getWidth(), area.getHeight()) * 0.44f;

    // hex nut, dark gunmetal
    juce::Path nut;
    for (int i = 0; i < 6; ++i)
    {
        const float a = static_cast<float> (i) / 6.0f * 2.0f * kPi + kPi / 6.0f;
        const auto p = c + juce::Point<float> (std::cos (a), std::sin (a)) * r;
        if (i == 0) nut.startNewSubPath (p);
        else        nut.lineTo (p);
    }
    nut.closeSubPath();

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (c.x - r * 1.02f, c.y - r * 0.95f, r * 2.04f, r * 1.9f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5c5f63), c.x - r * 0.5f, c.y - r,
                                             juce::Colour (0xff1a1a1c), c.x + r * 0.5f, c.y + r, false));
    g.fillPath (nut);
    g.setColour (juce::Colours::black.withAlpha (0.75f));
    g.strokePath (nut, juce::PathStrokeType (0.9f));
    {
        juce::Path hi;
        hi.addCentredArc (c.x, c.y, r * 0.96f, r * 0.96f, 0.0f, -2.5f, -0.7f, true);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.strokePath (hi, juce::PathStrokeType (0.8f));
    }

    // chrome bat: down for OFF, up for ON
    const float len = r * (on ? 1.05f : 0.75f);
    const float tilt = on ? 0.0f : 0.12f;
    juce::Path bat;
    const float w = r * 0.42f;
    bat.startNewSubPath (c.x - w * 0.5f, c.y + r * 0.15f);
    bat.lineTo (c.x - w * 0.33f + tilt * len, c.y - len);
    bat.lineTo (c.x + w * 0.33f + tilt * len, c.y - len);
    bat.lineTo (c.x + w * 0.5f, c.y + r * 0.15f);
    bat.closeSubPath();

    g.setGradientFill (juce::ColourGradient (cAluHi, c.x - w, c.y - len,
                                             cAluLo, c.x + w, c.y, false));
    g.fillPath (bat);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.strokePath (bat, juce::PathStrokeType (0.7f));

    // ball tip
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, c.x - w * 0.4f + tilt * len, c.y - len - 2.5f,
                                             cAluMid, c.x + w * 0.55f + tilt * len, c.y - len + 3.5f, true));
    g.fillEllipse (c.x - w * 0.62f + tilt * len, c.y - len - w * 0.62f, w * 1.24f, w * 1.24f);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.drawEllipse (c.x - w * 0.62f + tilt * len, c.y - len - w * 0.62f, w * 1.24f, w * 1.24f, 0.6f);
}

//==============================================================================
void PanelRenderer::drawLamp (juce::Graphics& g, juce::Point<float> centre, float radius,
                              juce::Colour colour, float glow)
{
    const auto c = centre;
    const float r = radius;

    // brass ferrule
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (c.x - r * 1.35f, c.y - r * 1.25f, r * 2.7f, r * 2.7f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd9c3a2), c.x - r, c.y - r,
                                             juce::Colour (0xff5a4a38), c.x + r, c.y + r, true));
    g.fillEllipse (c.x - r * 1.18f, c.y - r * 1.18f, r * 2.36f, r * 2.36f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawEllipse (c.x - r * 1.18f, c.y - r * 1.18f, r * 2.36f, r * 2.36f, 0.6f);

    if (glow > 0.01f)
    {
        juce::ColourGradient halo (colour.withAlpha (0.42f * glow), c.x, c.y,
                                   colour.withAlpha (0.0f), c.x + r * 3.6f, c.y, true);
        g.setGradientFill (halo);
        g.fillEllipse (c.x - r * 3.6f, c.y - r * 3.6f, r * 7.2f, r * 7.2f);
    }

    // glass dome
    g.setGradientFill (juce::ColourGradient (colour.brighter (0.45f + 0.45f * glow),
                                                c.x - r * 0.35f, c.y - r * 0.45f,
                                                colour.darker (0.8f), c.x + r * 0.45f, c.y + r * 0.55f, true));
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);

    g.setColour (juce::Colours::white.withAlpha (0.20f + 0.55f * glow));
    g.fillEllipse (c.x - r * 0.5f, c.y - r * 0.6f, r * 0.45f, r * 0.32f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 0.8f);
}

//==============================================================================
void PanelRenderer::drawMeterNeedle (juce::Graphics& g, const PanelLayout& l, const MeterState& m)
{
    const auto geo = meterGeometry (l);
    const auto pivot = geo.pivot;
    const float r = geo.radius;

    float norm;
    if (m.grMode)
        norm = 1.0f - juce::jlimit (0.0f, 1.0f, m.grDb / 20.0f);   // 0 GR at the right
    else
        norm = posForDb (juce::jlimit (-20.0f, 3.0f, m.levelDb));

    const float a = angleFor (norm);
    const auto d = juce::Point<float> (std::sin (a), -std::cos (a));
    const auto tip = pivot + d * (r - 2.0f);

    // needle shadow
    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.drawLine ({ pivot + juce::Point<float> (1.2f, 1.4f), tip + juce::Point<float> (1.2f, 1.4f) }, 2.6f);

    g.setColour (juce::Colour (0xff22180e));
    g.drawLine ({ pivot, tip }, 1.8f);

    // red tip over the last part of the travel, as on a real VU
    g.setColour (cRed);
    g.drawLine ({ pivot + d * (r - 26.0f), tip }, 2.0f);

    // hub: a dark semicircle sitting on the bottom of the card
    {
        const float hr = r * 0.17f;
        juce::Path hub;
        hub.addPieSegment (pivot.x - hr, pivot.y - hr, hr * 2.0f, hr * 2.0f, -kPi * 0.5f, kPi * 0.5f, 0.62f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5a5248), pivot.x - hr * 0.4f, pivot.y - hr,
                                                 juce::Colour (0xff0b0906), pivot.x + hr * 0.4f, pivot.y, true));
        g.fillPath (hub);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.strokePath (hub, juce::PathStrokeType (0.7f));
    }

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff443c32), pivot.x - 2.6f, pivot.y - 2.6f,
                                             juce::Colour (0xff060505), pivot.x + 2.6f, pivot.y + 2.6f, true));
    g.fillEllipse (pivot.x - 2.4f, pivot.y - 2.4f, 4.8f, 4.8f);
}

void PanelRenderer::drawMeterGlass (juce::Graphics& g, const PanelLayout& l, const MeterState& m)
{
    const auto card = l.meterCard;

    if (m.powered)
    {
        juce::ColourGradient glow (juce::Colour (0x28ffd08a), card.getCentreX(), card.getBottom(),
                                   juce::Colours::transparentBlack, card.getCentreX(), card.getY(), true);
        g.setGradientFill (glow);
        g.fillRect (card);
    }

    juce::Path shine;
    shine.addQuadrilateral (card.getX(), card.getY(),
                            card.getX() + card.getWidth() * 0.38f, card.getY(),
                            card.getX() + card.getWidth() * 0.12f, card.getBottom(),
                            card.getX(), card.getBottom());
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.13f), card.getX(), card.getY(),
                                             juce::Colours::white.withAlpha (0.02f), card.getRight(), card.getBottom(), true));
    g.fillPath (shine);

    // inner shadow of the bezel lip, all round
    {
        juce::ColourGradient top (juce::Colours::black.withAlpha (0.45f), card.getCentreX(), card.getY(),
                                  juce::Colours::transparentBlack, card.getCentreX(), card.getY() + 9.0f, false);
        g.setGradientFill (top);
        g.fillRect (card.withHeight (9.0f));
        juce::ColourGradient bot (juce::Colours::transparentBlack, card.getCentreX(), card.getBottom() - 7.0f,
                                  juce::Colours::black.withAlpha (0.40f), card.getCentreX(), card.getBottom(), false);
        g.setGradientFill (bot);
        g.fillRect (card.withTop (card.getBottom() - 7.0f));
    }

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawRect (card, 0.9f);
}
} // namespace ember::ui
