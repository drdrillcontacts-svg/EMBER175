/*
    Offline front panel renderer.

    Builds the same graphics that the plug-in editor shows, without needing a
    display, so that the panel design can be reviewed in CI or on a headless
    machine:

        ember_ui_preview out.png [width] [height]

    This target is optional (EMBER_BUILD_UI_PREVIEW). Knobs are drawn through
    the same component entry point the editor uses, so the preview validates the
    editor's placement maths rather than a private copy of it.
*/

#include "../Source/UI/PanelRenderer.h"

#include <juce_graphics/juce_graphics.h>
#include <cstdio>
#include <cstdlib>

int main (int argc, char** argv)
{
    const juce::String outPath = argc > 1 ? juce::String (argv[1]) : juce::String ("ember175-panel.png");
    const int width = argc > 2 ? std::atoi (argv[2]) : 1800;
    const int height = argc > 3 ? std::atoi (argv[3]) : 540;

    juce::Image image (juce::Image::ARGB, width, height, true);
    juce::Graphics g (image);
    g.fillAll (juce::Colour (0xff141010));

    auto layout = ember::ui::makeLayout();
    const float scale = std::min (static_cast<float> (width) / ember::ui::kDesignWidth,
                                  static_cast<float> (height) / ember::ui::kDesignHeight);
    const float ox = (static_cast<float> (width) - ember::ui::kDesignWidth * scale) * 0.5f;
    const float oy = (static_cast<float> (height) - ember::ui::kDesignHeight * scale) * 0.5f;

    g.saveState();
    g.addTransform (juce::AffineTransform().scaled (scale).translated (ox, oy));
    ember::ui::PanelRenderer::renderBackground (g, layout, 1.0f);

    struct Demo { int index; float norm; };
    const Demo demo[] = {
        { 0, 0.25f },   // low / hi
        { 1, 0.72f },   // input
        { 2, 1.00f },   // impedance 300
        { 3, 0.55f },   // vernier
        { 4, 0.78f },   // attack
        { 5, 0.58f },   // output
        { 6, 1.00f },   // mix
        { 7, 0.52f }    // release
    };

    for (auto d : demo)
    {
        if (auto* k = layout.knobFor (d.index))
        {
            const auto area = juce::Rectangle<float> (k->centre.x - k->radius * 1.3f,
                                                      k->centre.y - k->radius * 1.3f,
                                                      k->radius * 2.6f, k->radius * 2.6f);
            ember::ui::PanelRenderer::paintKnobComponent (g, *k, area, d.norm, false, d.index == 5);
        }
    }

    ember::ui::MeterState meter;
    meter.grDb = 0.0f;
    meter.levelDb = -4.5f;
    meter.grMode = false;
    meter.powered = true;
    ember::ui::PanelRenderer::drawMeterNeedle (g, layout, meter);
    ember::ui::PanelRenderer::drawMeterGlass (g, layout, meter);

    ember::ui::PanelRenderer::drawToggle (g, layout.linkToggle, true, "S.C. LINK");
    ember::ui::PanelRenderer::drawToggle (g, layout.powerToggle, true, "POWER");

    ember::ui::PanelRenderer::drawLamp (g, layout.lamp, layout.lampRadius, juce::Colour (0xffffa53a), 0.75f);
    ember::ui::PanelRenderer::drawLamp (g, layout.powerLamp, layout.powerLampRadius, juce::Colour (0xffff3a24), 1.0f);

    ember::ui::PanelRenderer::drawStatus (g, layout.statusLine,
                                          "GR 3.2 dB     OUT -7.4 dBFS     ATTACK 0.30 ms     RELEASE 120 ms");

    g.restoreState();

    juce::File out (outPath);
    out.deleteFile();
    juce::FileOutputStream stream (out);
    juce::PNGImageFormat png;
    png.writeImageToStream (image, stream);

    std::printf ("wrote %s (%d x %d)\n", outPath.toRawUTF8(), width, height);
    return 0;
}
