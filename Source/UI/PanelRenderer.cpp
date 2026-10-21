#include "PanelRenderer.h"

namespace ember::ui
{
namespace
{
    constexpr float kPi = juce::MathConstants<float>::pi;

    // ------------------------------------------------------------------ palette
    const juce::Colour cPanelTop   { 0xff4a2229 };
    const juce::Colour cPanelBot   { 0xff281217 };
    const juce::Colour cPrint      { 0xffe8ddcf };
    const juce::Colour cPrintDim   { 0xffb9a595 };
    const juce::Colour cChromeHi   { 0xfff2f3f5 };
    const juce::Colour cChromeMid  { 0xffb8bcc1 };
    const juce::Colour cChromeLo   { 0xff6b6f75 };
    const juce::Colour cChromeDark { 0xff3a3c40 };
    const juce::Colour cInk        { 0xff241a12 };
    const juce::Colour cRed        { 0xffb62a1e };
    const juce::Colour cCream      { 0xffefd9a6 };

    void text (juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r,
               float height, juce::Colour colour,
               juce::Justification just = juce::Justification::centred,
               bool bold = true, float kerning = 0.2f)
    {
        g.setColour (colour);
        g.setFont (PanelRenderer::panelFont (height, bold, kerning));
        g.drawText (s, r, just, false);
    }

    /** Radial brushed aluminium, used for every knob body. */
    void brushedDisc (juce::Graphics& g, juce::Point<float> c, float r, float spin)
    {
        juce::ColourGradient grad (juce::Colour (0xfff6f7f8), c.x - r * 0.35f, c.y - r * 0.45f,
                                   juce::Colour (0xff8b9096), c.x + r * 0.45f, c.y + r * 0.55f, true);
        g.setGradientFill (grad);
        g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);

        // fine radial brushing
        const int lines = 150;
        for (int i = 0; i < lines; ++i)
        {
            const float a = static_cast<float> (i) / static_cast<float> (lines) * 2.0f * kPi + spin;
            const auto d = juce::Point<float> (std::cos (a), std::sin (a));
            const float alpha = 0.02f + 0.05f * std::abs (std::sin (a * 3.0f + spin));
            g.setColour (juce::Colours::white.withAlpha (alpha));
            g.drawLine ({ c + d * (r * 0.18f), c + d * (r * 0.98f) }, 0.7f);
        }

        // knurled rim
        const int flutes = static_cast<int> (juce::jlimit (44.0f, 150.0f, r * 2.6f));
        for (int i = 0; i < flutes; ++i)
        {
            const float a = static_cast<float> (i) / static_cast<float> (flutes) * 2.0f * kPi + spin;
            const auto d = juce::Point<float> (std::cos (a), std::sin (a));
            g.setColour (juce::Colours::black.withAlpha (i % 2 == 0 ? 0.22f : 0.08f));
            g.drawLine ({ c + d * (r * 0.90f), c + d * r }, 1.1f);
            g.setColour (juce::Colours::white.withAlpha (i % 2 == 0 ? 0.14f : 0.05f));
            g.drawLine ({ c + d * (r * 0.86f), c + d * (r * 0.90f) }, 0.9f);
        }

        // bright chrome rim: light on the top left, shadow on the bottom right
        {
            juce::Path hiArc;
            hiArc.addCentredArc (c.x, c.y, r * 0.985f, r * 0.985f, 0.0f, -2.75f, -0.35f, true);
            g.setColour (juce::Colours::white.withAlpha (0.65f));
            g.strokePath (hiArc, juce::PathStrokeType (1.6f));

            juce::Path loArc;
            loArc.addCentredArc (c.x, c.y, r * 0.985f, r * 0.985f, 0.0f, 0.25f, 2.65f, true);
            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.strokePath (loArc, juce::PathStrokeType (1.4f));
        }

        // specular sweep across the dome
        {
            juce::Path glare;
            glare.addCentredArc (c.x, c.y, r * 0.62f, r * 0.62f, 0.0f, -2.4f, -1.1f, true);
            g.setColour (juce::Colours::white.withAlpha (0.30f));
            g.strokePath (glare, juce::PathStrokeType (r * 0.16f));
        }

        // top light and bottom shade
        juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.35f), c.x - r * 0.3f, c.y - r * 0.75f,
                                    juce::Colours::transparentBlack, c.x, c.y + r * 0.2f, true);
        g.setGradientFill (sheen);
        g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
        juce::ColourGradient shade (juce::Colours::transparentBlack, c.x, c.y - r * 0.1f,
                                    juce::Colours::black.withAlpha (0.35f), c.x, c.y + r, true);
        g.setGradientFill (shade);
        g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);

        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 1.0f);
    }

    void pointer (juce::Graphics& g, juce::Point<float> c, float r, float angle,
                  juce::Colour colour, float width, float from)
    {
        const auto d = juce::Point<float> (std::sin (angle), -std::cos (angle));
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawLine ({ c + d * (r * from), c + d * (r * 0.97f) }, width + 1.4f);
        g.setColour (colour);
        g.drawLine ({ c + d * (r * from), c + d * (r * 0.97f) }, width);
    }

    // ------------------------------------------------------------------ panel
    void paintFace (juce::Graphics& g, const PanelLayout& l)
    {
        const auto face = l.face;

        // metal enclosure: brushed cheeks and a chrome edge
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8d9196), 0.0f, 0.0f,
                                                 juce::Colour (0xff4a4d52), 0.0f, kDesignHeight, false));
        g.fillRoundedRectangle (l.panel, 7.0f);

        g.setGradientFill (juce::ColourGradient (cChromeHi, face.getX(), face.getY(),
                                                 cChromeLo, face.getX(), face.getBottom(), false));
        g.fillRoundedRectangle (face, 4.0f);

        auto inner = face.reduced (2.0f);
        g.setGradientFill (juce::ColourGradient (cPanelTop, inner.getCentreX(), inner.getY(),
                                                 cPanelBot, inner.getCentreX(), inner.getBottom(), false));
        g.fillRoundedRectangle (inner, 3.0f);

        // warm light pooling around the meter
        {
            juce::ColourGradient glow (juce::Colour (0x33b0552b), l.meter.getCentreX(), l.meter.getBottom(),
                                       juce::Colours::transparentBlack, l.meter.getCentreX(), l.meter.getBottom() + 150.0f, true);
            g.setGradientFill (glow);
            g.fillRoundedRectangle (inner, 3.0f);
        }

        // subtle horizontal brushing and vignette
        juce::Random rng (0x175D);
        for (int i = 0; i < 320; ++i)
        {
            const float y = inner.getY() + rng.nextFloat() * inner.getHeight();
            g.setColour (rng.nextBool() ? juce::Colours::white.withAlpha (0.012f + rng.nextFloat() * 0.02f)
                                        : juce::Colours::black.withAlpha (0.02f + rng.nextFloat() * 0.03f));
            g.fillRect (inner.getX(), y, inner.getWidth(), 1.0f);
        }
        {
            juce::ColourGradient vig (juce::Colours::transparentBlack, inner.getCentreX(), inner.getCentreY(),
                                      juce::Colours::black.withAlpha (0.35f), inner.getX(), inner.getY(), true);
            g.setGradientFill (vig);
            g.fillRoundedRectangle (inner, 3.0f);
        }

        g.setColour (juce::Colours::white.withAlpha (0.16f));
        g.drawRoundedRectangle (inner.reduced (0.5f), 3.0f, 1.0f);
    }

    void hairline (juce::Graphics& g, float x1, float x2, float y)
    {
        juce::ColourGradient line (juce::Colours::transparentBlack, x1, y,
                                   cChromeHi.withAlpha (0.75f), (x1 + x2) * 0.5f, y, false);
        juce::ColourGradient full (juce::Colours::transparentBlack, x1, y,
                                   cChromeHi.withAlpha (0.75f), (x1 + x2) * 0.5f, y, false);
        full.addColour (1.0, juce::Colours::transparentBlack);
        g.setGradientFill (full);
        g.fillRect (x1, y, x2 - x1, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRect (x1, y + 1.0f, x2 - x1, 1.0f);
    }

    // ------------------------------------------------------------------ meter
    // Classic VU scale: the marks are placed where a real moving-coil meter
    // prints them, which compresses -20..-5 and expands the last few dB around
    // zero so the numbers stay readable.
    struct VUPoint { int db; float pos; };
    static const VUPoint vuScale[] = {
        { -20, 0.000f }, { -15, 0.130f }, { -10, 0.260f }, { -7, 0.395f },
        {  -5, 0.500f }, {  -3, 0.600f }, {  -2, 0.660f }, { -1, 0.730f },
        {   0, 0.800f }, {   1, 0.868f }, {   2, 0.932f }, {  3, 1.000f }
    };

    static float posForDb (int db)
    {
        const auto* table = vuScale;
        constexpr int n = static_cast<int> (sizeof (vuScale) / sizeof (vuScale[0]));
        if (db <= table[0].db) return table[0].pos;
        if (db >= table[n - 1].db) return table[n - 1].pos;
        for (int i = 1; i < n; ++i)
        {
            if (db <= table[i].db)
            {
                const float t = static_cast<float> (db - table[i - 1].db)
                              / static_cast<float> (table[i].db - table[i - 1].db);
                return table[i - 1].pos + t * (table[i].pos - table[i - 1].pos);
            }
        }
        return 1.0f;
    }

    static float posForDb (float db)
    {
        const int lo = static_cast<int> (std::floor (db));
        const int hi = static_cast<int> (std::ceil (db));
        if (lo == hi)
            return posForDb (lo);
        return posForDb (lo) + (db - static_cast<float> (lo)) * (posForDb (hi) - posForDb (lo));
    }

    void paintMeterCard (juce::Graphics& g, const PanelLayout& l)
    {
        const auto bezel = l.meter;
        const auto face = l.meterFace;

        // bezel
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (bezel.translated (0.0f, 2.0f), 5.0f);
        g.setGradientFill (juce::ColourGradient (cChromeMid, bezel.getX(), bezel.getY(),
                                                 cChromeDark, bezel.getRight(), bezel.getBottom(), false));
        g.fillRoundedRectangle (bezel, 4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.65f));
        g.drawRoundedRectangle (bezel, 4.0f, 1.2f);
        g.setColour (juce::Colours::white.withAlpha (0.22f));
        g.drawRoundedRectangle (bezel.reduced (1.4f), 3.0f, 0.8f);

        // warm card
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff3dfae), face.getCentreX(), face.getY(),
                                                 juce::Colour (0xffd9b676), face.getCentreX(), face.getBottom(), false));
        g.fillRect (face);

        // lamp glow from the bottom of the card
        {
            juce::ColourGradient glow (juce::Colour (0x88ffb552), face.getCentreX(), face.getBottom(),
                                       juce::Colours::transparentBlack, face.getCentreX(), face.getY(), true);
            g.setGradientFill (glow);
            g.fillRect (face);
        }

        juce::Random rng (0x175E);
        for (int i = 0; i < 260; ++i)
        {
            g.setColour (juce::Colour (0xff8c6230).withAlpha (0.02f + rng.nextFloat() * 0.05f));
            g.fillEllipse (face.getX() + rng.nextFloat() * face.getWidth(),
                           face.getY() + rng.nextFloat() * face.getHeight(), 1.6f, 1.6f);
        }

        g.setColour (juce::Colours::black.withAlpha (0.30f));
        g.drawRect (face, 1.0f);

        // ---- scale --------------------------------------------------------
        const auto pivot = juce::Point<float> (face.getCentreX(), face.getBottom() - 6.0f);
        const float h = face.getHeight();
        const float r = h * 0.66f;
        const float sweep = juce::MathConstants<float>::pi * 0.30f;

        auto angleFor = [sweep] (float n)
        {
            n = juce::jlimit (0.0f, 1.0f, n);
            return -sweep + n * 2.0f * sweep;
        };

        // printed arc
        {
            juce::Path arc;
            arc.addCentredArc (pivot.x, pivot.y, r, r, 0.0f, angleFor (0.0f), angleFor (1.0f), true);
            g.setColour (cInk.withAlpha (0.85f));
            g.strokePath (arc, juce::PathStrokeType (1.2f));
        }

        // red zone, 0 dB to +3 dB, printed as a band just outside the arc
        {
            const float n0 = posForDb (0), n1 = posForDb (3);
            juce::Path band;
            band.addCentredArc (pivot.x, pivot.y, r + 3.4f, r + 3.4f, 0.0f, angleFor (n0), angleFor (n1), true);
            g.setColour (cRed.withAlpha (0.92f));
            g.strokePath (band, juce::PathStrokeType (3.6f));
        }

        struct Mark { int db; bool label; };
        const Mark marks[] = { { -20, true }, { -15, false }, { -10, true }, { -7, true }, { -5, true },
                               { -3, true }, { -2, false }, { -1, false }, { 0, true },
                               { 1, false }, { 2, false }, { 3, true } };

        for (auto m : marks)
        {
            const float n = posForDb (m.db);
            const auto a = angleFor (n);
            const auto d = juce::Point<float> (std::sin (a), -std::cos (a));
            const bool red = m.db > 0;
            g.setColour (red ? cRed : cInk);
            g.drawLine ({ pivot + d * (r - 5.0f), pivot + d * (r + 1.5f) }, m.label ? 1.4f : 1.0f);

            if (! m.label)
                continue;

            const auto p = pivot + d * (r - (red ? 13.0f : 14.0f));
            g.setFont (PanelRenderer::panelFont (m.db >= 0 ? 6.6f : 7.2f, true, 0.0f));
            g.setColour (red ? cRed : cInk);
            if (red)
            {
                // the red end of the scale is printed upright, clear of the band
                g.drawText ("+" + juce::String (m.db),
                            juce::Rectangle<float> (p.x - 11.0f, p.y - 5.0f, 22.0f, 10.0f),
                            juce::Justification::centred, false);
            }
            else
            {
                g.saveState();
                g.addTransform (juce::AffineTransform::rotation (a, p.x, p.y));
                g.drawText (juce::String (m.db),
                            juce::Rectangle<float> (p.x - 11.0f, p.y - 6.0f, 22.0f, 12.0f),
                            juce::Justification::centred, false);
                g.restoreState();
            }
        }

        // "VU" and "dB" legends
        text (g, "VU", { pivot.x - 14.0f, pivot.y - 16.0f, 28.0f, 12.0f },
              8.0f, cInk.withAlpha (0.85f), juce::Justification::centred, true, 0.1f);
        text (g, "dB", { face.getRight() - 30.0f, pivot.y - 16.0f, 22.0f, 12.0f },
              8.0f, cInk.withAlpha (0.85f), juce::Justification::centred, true, 0.05f);
    }

    void paintBrand (juce::Graphics& g, const PanelLayout& l)
    {
        const auto c = l.brandLogo;
        const float r = l.brandLogoRadius;

        juce::Path diamond;
        diamond.addQuadrilateral (c.x, c.y - r, c.x + r, c.y, c.x, c.y + r, c.x - r, c.y);
        g.setColour (cChromeHi.withAlpha (0.85f));
        g.strokePath (diamond, juce::PathStrokeType (1.4f));
        juce::Path inner;
        inner.addQuadrilateral (c.x, c.y - r + 4.0f, c.x + r - 4.0f, c.y, c.x, c.y + r - 4.0f, c.x - r + 4.0f, c.y);
        g.setColour (cChromeHi.withAlpha (0.5f));
        g.strokePath (inner, juce::PathStrokeType (0.8f));

        text (g, "E", { c.x - r, c.y - r, r * 2.0f, r * 2.0f }, r * 0.75f, cChromeHi,
              juce::Justification::centred, true, 0.0f);

        text (g, "EMBER 175", l.brandName, 18.0f, cPrint, juce::Justification::centred, false, 0.30f);
        text (g, "TUBE COMPRESSOR", l.brandSub, 7.6f, cPrintDim, juce::Justification::centred, false, 0.55f);

        // rules either side of the wordmark, as on the reference panel
        const float y = l.brandSub.getCentreY();
        g.setColour (cChromeHi.withAlpha (0.35f));
        g.drawLine (l.brandName.getX() - 44.0f, y, l.brandName.getX() - 6.0f, y, 0.8f);
        g.drawLine (l.brandName.getRight() + 6.0f, y, l.brandName.getRight() + 44.0f, y, 0.8f);
    }
}

//==============================================================================
juce::Font PanelRenderer::panelFont (float height, bool bold, float kerning)
{
    juce::Font f (juce::FontOptions (height).withStyle (bold ? "Bold" : "Regular"));
    f.setExtraKerningFactor (kerning);
    return f;
}

juce::Colour PanelRenderer::faceDark()   { return cPanelBot; }
juce::Colour PanelRenderer::print()      { return cPrint; }
juce::Colour PanelRenderer::printMuted() { return cPrintDim; }
juce::Colour PanelRenderer::brass()      { return cChromeMid; }

//==============================================================================
void PanelRenderer::renderBackground (juce::Graphics& g, const PanelLayout& l, float scale)
{
    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::scale (scale));

    g.setColour (juce::Colour (0xff0f0c0c));
    g.fillRect (l.panel);

    paintFace (g, l);
    paintMeterCard (g, l);
    paintBrand (g, l);

    // control captions and printed scales
    auto caption = [] (juce::Graphics& gg, const juce::String& s, juce::Point<float> centre, float dz)
    {
        if (s.isEmpty())
            return;
        text (gg, s, { centre.x - 70.0f, centre.y - dz, 140.0f, 12.0f }, 7.8f, cPrint,
              juce::Justification::centred, true, 0.28f);
    };

    for (int i = 0; i < PanelLayout::numKnobs; ++i)
    {
        if (auto* k = const_cast<PanelLayout&> (l).knobFor (i))
        {
            // ticks
            if (k->tickCount > 1)
            {
                for (int t = 0; t < k->tickCount; ++t)
                {
                    const float n = static_cast<float> (t) / static_cast<float> (k->tickCount - 1);
                    const float a = k->angleMin + n * (k->angleMax - k->angleMin);
                    const auto d = juce::Point<float> (std::sin (a), -std::cos (a));
                    const bool major = k->style == KnobStyle::big || (t % 2 == 0);
                    g.setColour (cPrint.withAlpha (major ? 0.85f : 0.45f));
                    g.drawLine ({ k->centre + d * (k->ringRadius - (major ? 5.0f : 3.0f)),
                                  k->centre + d * k->ringRadius },
                                major ? 1.3f : 0.9f);
                }
            }

            // printed numbers
            if (! k->tickLabels.isEmpty())
            {
                const int count = k->tickLabels.size();
                for (int t = 0; t < count; ++t)
                {
                    const float n = count > 1 ? static_cast<float> (t) / static_cast<float> (count - 1) : 0.0f;
                    const float a = k->angleMin + n * (k->angleMax - k->angleMin);
                    const float rad = k->ringRadius + (k->style == KnobStyle::big ? 13.0f : 12.0f);
                    const auto p = k->centre + juce::Point<float> (std::sin (a), -std::cos (a)) * rad;
                    g.setColour (cPrint);
                    g.setFont (panelFont (k->style == KnobStyle::big ? 9.2f : 7.4f, true, 0.0f));
                    g.drawText (k->tickLabels[t],
                                juce::Rectangle<float> (p.x - 13.0f, p.y - 6.0f, 26.0f, 12.0f),
                                juce::Justification::centred, false);
                }
            }

            // side labels (two position switches and verniers)
            if (k->leftLabel.isNotEmpty())
                text (g, k->leftLabel, { k->centre.x - k->ringRadius - 34.0f, k->centre.y - 6.0f, 30.0f, 12.0f },
                      7.6f, cPrint, juce::Justification::centredRight, true, 0.12f);
            if (k->rightLabel.isNotEmpty())
                text (g, k->rightLabel, { k->centre.x + k->ringRadius + 4.0f, k->centre.y - 6.0f, 30.0f, 12.0f },
                      7.6f, cPrint, juce::Justification::centredLeft, true, 0.12f);

            // caption, clear of the printed numbers
            if (k->label.isNotEmpty())
            {
                const float extra = k->tickLabels.isEmpty() ? 17.0f : 34.0f;
                const float capY = k->captionAbove ? k->centre.y - k->ringRadius - extra
                                                   : k->centre.y + k->ringRadius + 9.0f;
                const float capSize = k->style == KnobStyle::big ? 8.8f : 7.8f;
                text (g, k->label, { k->centre.x - 80.0f, capY, 160.0f, 12.0f },
                      capSize, cPrint, juce::Justification::centred, true, 0.32f);
            }

            // OFF legend, under the anticlockwise end of the dial
            if (k->offLabel.isNotEmpty())
                text (g, k->offLabel,
                      { k->centre.x - k->ringRadius - 26.0f, k->centre.y + k->ringRadius + 14.0f, 44.0f, 11.0f },
                      7.2f, cPrintDim, juce::Justification::centred, true, 0.18f);
        }
    }

    // S.C. LINK and POWER captions
    text (g, "S.C. LINK", { l.linkToggle.getCentreX() - 50.0f, l.linkToggle.getY() - 46.0f, 100.0f, 12.0f },
          7.6f, cPrint, juce::Justification::centred, true, 0.22f);
    text (g, "OFF", { l.linkOffLabel.x - 24.0f, l.linkOffLabel.y - 6.0f, 48.0f, 12.0f },
          7.4f, cPrintDim, juce::Justification::centred, true, 0.15f);
    text (g, "ON", { l.linkOnLabel.x - 24.0f, l.linkOnLabel.y - 6.0f, 48.0f, 12.0f },
          7.4f, cPrint, juce::Justification::centred, true, 0.15f);

    text (g, "POWER", { l.powerToggle.getCentreX() - 50.0f, l.powerToggle.getY() - 20.0f, 100.0f, 12.0f },
          7.6f, cPrint, juce::Justification::centred, true, 0.22f);
    text (g, "ON", { l.powerOnLabel.x - 24.0f, l.powerOnLabel.y - 6.0f, 48.0f, 12.0f },
          7.4f, cPrint, juce::Justification::centred, true, 0.15f);

    // hairlines under the control groups
    hairline (g, l.face.getX() + 26.0f, 452.0f, 300.0f);
    hairline (g, 790.0f, l.face.getRight() - 26.0f, 300.0f);
}

//==============================================================================
void PanelRenderer::paintKnobComponent (juce::Graphics& g, const KnobSpec& k, juce::Rectangle<float> localArea,
                                        float normalised, bool highlighted, bool dragging)
{
    if (localArea.isEmpty())
        return;

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

    // cast shadow on the panel; it has to be fully transparent by r * 1.25 so
    // that it never reaches the edge of the knob component and gets clipped
    {
        juce::ColourGradient sh (juce::Colours::black.withAlpha (0.55f), c.x, c.y + r * 0.45f,
                                 juce::Colours::transparentBlack, c.x, c.y + r * 1.22f, true);
        g.setGradientFill (sh);
        g.fillEllipse (c.x - r * 1.2f, c.y - r * 0.5f, r * 2.4f, r * 2.6f);
    }

    if (k.style == KnobStyle::big)
    {
        // skirt
        brushedDisc (g, c, r * 1.06f, 0.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawEllipse (c.x - r * 1.06f, c.y - r * 1.06f, r * 2.12f, r * 2.12f, 1.0f);

        // domed cap: slightly darker brushed metal, machined step around it
        const float ir = r * 0.70f;
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.fillEllipse (c.x - ir - 2.0f, c.y - ir - 2.0f, (ir + 2.0f) * 2.0f, (ir + 2.0f) * 2.0f);

        juce::ColourGradient dome (juce::Colour (0xffd9dcdf), c.x - ir * 0.35f, c.y - ir * 0.5f,
                                   juce::Colour (0xff6d7278), c.x + ir * 0.45f, c.y + ir * 0.62f, true);
        g.setGradientFill (dome);
        g.fillEllipse (c.x - ir, c.y - ir, ir * 2.0f, ir * 2.0f);

        for (int i = 0; i < 90; ++i)
        {
            const float a = static_cast<float> (i) / 90.0f * 2.0f * kPi;
            const auto d = juce::Point<float> (std::cos (a), std::sin (a));
            g.setColour (juce::Colours::white.withAlpha (0.03f + 0.05f * std::abs (std::sin (a))));
            g.drawLine ({ c + d * (ir * 0.14f), c + d * (ir * 0.97f) }, 0.6f);
        }

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawEllipse (c.x - ir, c.y - ir, ir * 2.0f, ir * 2.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawEllipse (c.x - ir * 0.97f, c.y - ir * 0.97f, ir * 1.94f, ir * 1.94f, 0.7f);

        pointer (g, c, ir * 1.02f, angle, juce::Colour (0xfffbfbfb), 2.0f, 0.04f);
        g.setColour (juce::Colour (0xff2a2a2b));
        g.fillEllipse (c.x - 2.2f, c.y - 2.2f, 4.4f, 4.4f);
    }
    else
    {
        brushedDisc (g, c, r, 0.35f);
        pointer (g, c, r * 0.92f, angle, juce::Colour (0xff1d1a17), 2.0f, 0.10f);

        if (k.style == KnobStyle::small)
        {
            // two position switches carry a small centre dot
            g.setColour (juce::Colour (0xffd9dadd));
            g.fillEllipse (c.x - 2.1f, c.y - 2.1f, 4.2f, 4.2f);
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.drawEllipse (c.x - 2.1f, c.y - 2.1f, 4.2f, 4.2f, 0.7f);
        }
    }

    if (highlighted || dragging)
    {
        g.setColour (juce::Colour (0xfff2d9a0).withAlpha (dragging ? 0.55f : 0.25f));
        g.drawEllipse (c.x - r * 1.12f, c.y - r * 1.12f, r * 2.24f, r * 2.24f, dragging ? 1.8f : 1.0f);
    }
}

//==============================================================================
void PanelRenderer::drawToggle (juce::Graphics& g, juce::Rectangle<float> area, bool on,
                                const juce::String& label, bool showLabel)
{
    juce::ignoreUnused (label, showLabel);

    const auto c = juce::Point<float> (area.getCentreX(), area.getY() + area.getHeight() * 0.62f);
    const float baseR = area.getWidth() * 0.46f;

    // threaded nut base
    juce::Path nut;
    const int sides = 6;
    for (int i = 0; i < sides; ++i)
    {
        const float a = static_cast<float> (i) / sides * 2.0f * kPi + kPi / 6.0f;
        const auto p = c + juce::Point<float> (std::cos (a), std::sin (a) * 0.55f) * baseR;
        if (i == 0) nut.startNewSubPath (p);
        else        nut.lineTo (p);
    }
    nut.closeSubPath();

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (c.x - baseR, c.y - baseR * 0.62f + 1.5f, baseR * 2.0f, baseR * 1.3f);
    g.setGradientFill (juce::ColourGradient (cChromeHi, c.x - baseR * 0.5f, c.y - baseR,
                                             cChromeLo, c.x + baseR * 0.5f, c.y + baseR, true));
    g.fillPath (nut);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.strokePath (nut, juce::PathStrokeType (0.8f));

    // lever
    const float len = area.getHeight() * (on ? 0.60f : 0.46f);
    const float lean = on ? 0.0f : 0.0f;
    juce::Path lever;
    const float w = baseR * 0.62f;
    lever.startNewSubPath (c.x - w * 0.5f + lean, c.y);
    lever.lineTo (c.x - w * 0.34f, c.y - len);
    lever.lineTo (c.x + w * 0.34f, c.y - len);
    lever.lineTo (c.x + w * 0.5f + lean, c.y);
    lever.closeSubPath();

    g.setGradientFill (juce::ColourGradient (cChromeHi, c.x - w, c.y - len,
                                             cChromeLo, c.x + w, c.y, false));
    g.fillPath (lever);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.strokePath (lever, juce::PathStrokeType (0.7f));

    // ball tip
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, c.x - w * 0.4f, c.y - len - 3.0f,
                                             cChromeMid, c.x + w * 0.5f, c.y - len + 4.0f, true));
    g.fillEllipse (c.x - w * 0.55f, c.y - len - w * 0.55f, w * 1.1f, w * 1.1f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawEllipse (c.x - w * 0.55f, c.y - len - w * 0.55f, w * 1.1f, w * 1.1f, 0.7f);

    if (on)
    {
        g.setColour (juce::Colour (0x33ffd27a));
        g.fillEllipse (c.x - baseR * 1.4f, c.y - baseR * 1.0f, baseR * 2.8f, baseR * 1.8f);
    }
}

void PanelRenderer::drawLamp (juce::Graphics& g, juce::Point<float> centre, float radius,
                              juce::Colour colour, float glow)
{
    const auto c = centre;

    // chrome bezel
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (c.x - radius - 4.0f, c.y - radius - 3.0f, (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f);
    g.setGradientFill (juce::ColourGradient (cChromeHi, c.x - radius, c.y - radius,
                                             cChromeLo, c.x + radius, c.y + radius, true));
    g.fillEllipse (c.x - radius - 3.0f, c.y - radius - 3.0f, (radius + 3.0f) * 2.0f, (radius + 3.0f) * 2.0f);

    if (glow > 0.01f)
    {
        juce::ColourGradient halo (colour.withAlpha (0.45f * glow), c.x, c.y,
                                   colour.withAlpha (0.0f), c.x + radius * 3.4f, c.y, true);
        g.setGradientFill (halo);
        g.fillEllipse (c.x - radius * 3.4f, c.y - radius * 3.4f, radius * 6.8f, radius * 6.8f);
    }

    g.setGradientFill (juce::ColourGradient (colour.brighter (0.35f + 0.45f * glow), c.x - radius * 0.4f, c.y - radius * 0.5f,
                                             colour.darker (0.75f), c.x + radius * 0.5f, c.y + radius * 0.6f, true));
    g.fillEllipse (c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.25f + 0.45f * glow));
    g.fillEllipse (c.x - radius * 0.45f, c.y - radius * 0.55f, radius * 0.45f, radius * 0.32f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawEllipse (c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f, 0.9f);
}

//==============================================================================
void PanelRenderer::drawMeterNeedle (juce::Graphics& g, const PanelLayout& l, const MeterState& m)
{
    const auto face = l.meterFace;
    const auto pivot = juce::Point<float> (face.getCentreX(), face.getBottom() - 6.0f);
    const float r = face.getHeight() * 0.68f;
    const float sweep = juce::MathConstants<float>::pi * 0.30f;

    float norm;
    if (m.grMode)
    {
        norm = 1.0f - juce::jlimit (0.0f, 1.0f, m.grDb / 20.0f);   // 0 GR at the right
    }
    else
    {
        const float db = juce::jlimit (-20.0f, 3.0f, m.levelDb);
        norm = posForDb (db);
    }

    const float a = -sweep + norm * sweep * 2.0f;
    const auto d = juce::Point<float> (std::sin (a), -std::cos (a));
    const auto tip = pivot + d * (r - 2.0f);

    g.setColour (juce::Colours::black.withAlpha (0.20f));
    g.drawLine ({ pivot + juce::Point<float> (1.0f, 1.2f), tip + juce::Point<float> (1.0f, 1.2f) }, 2.2f);
    g.setColour (juce::Colour (0xff231a12));
    g.drawLine ({ pivot, tip }, 1.5f);

    // red tip
    g.setColour (cRed);
    g.drawLine ({ pivot + d * (r - 22.0f), tip }, 1.9f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3632), pivot.x - 7.0f, pivot.y - 7.0f,
                                             juce::Colour (0xff050505), pivot.x + 7.0f, pivot.y + 7.0f, true));
    g.fillEllipse (pivot.x - 6.5f, pivot.y - 6.5f, 13.0f, 13.0f);
    g.setColour (juce::Colours::white.withAlpha (0.2f));
    g.fillEllipse (pivot.x - 3.0f, pivot.y - 3.5f, 2.4f, 1.8f);
}

void PanelRenderer::drawMeterGlass (juce::Graphics& g, const PanelLayout& l, const MeterState& m)
{
    const auto face = l.meterFace;

    if (m.powered)
    {
        juce::ColourGradient glow (juce::Colour (0x30ffc070), face.getCentreX(), face.getBottom(),
                                   juce::Colours::transparentBlack, face.getCentreX(), face.getY(), true);
        g.setGradientFill (glow);
        g.fillRect (face);
    }

    juce::Path shine;
    shine.addQuadrilateral (face.getX(), face.getY(),
                            face.getX() + face.getWidth() * 0.42f, face.getY(),
                            face.getX() + face.getWidth() * 0.16f, face.getBottom(),
                            face.getX(), face.getBottom());
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.20f), face.getX(), face.getY(),
                                             juce::Colours::white.withAlpha (0.02f), face.getRight(), face.getBottom(), true));
    g.fillPath (shine);

    g.setColour (juce::Colours::black.withAlpha (0.30f));
    g.drawRect (face, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawLine (face.getX() + 1.0f, face.getY() + 1.0f, face.getRight() - 1.0f, face.getY() + 1.0f, 1.0f);
}

void PanelRenderer::drawStatus (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& s)
{
    if (s.isEmpty())
        return;
    g.setColour (juce::Colour (0xff2a1216).withAlpha (0.55f));
    g.fillRoundedRectangle (area.reduced (0.0f, 1.0f), 3.0f);
    text (g, s, area, 7.6f, juce::Colour (0xffd8c8b2).withAlpha (0.85f), juce::Justification::centred, false, 0.12f);
}
}
