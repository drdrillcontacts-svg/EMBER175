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
    const int height = argc > 3 ? std::atoi (argv[3]) : 360;

    juce::Image image (juce::Image::ARGB, width, height, true);
    juce::Graphics g (image);
    g.fillAll (juce::Colour (0xff0b0909));

    auto layout = ember::ui::makeLayout();
    const auto mapping = ember::ui::PanelMapping::fit (width, height);

    // Layout sanity check: every knob must land inside the rendered window at a
    // range of sizes. This is the check that would have caught the knobs being
    // drawn off-screen, so CI now fails loudly instead of shipping a blank panel.
    {
        const int sizes[][2] = { { width, height }, { 650, 130 }, { 1234, 500 }, { 2540, 1220 } };
        for (auto& s : sizes)
        {
            const auto m = ember::ui::PanelMapping::fit (s[0], s[1]);
            for (int i = 0; i < ember::ui::PanelLayout::numKnobs; ++i)
            {
                auto* k = layout.knobFor (i);
                if (k == nullptr)
                    continue;

                const float half = k->skirtRadius * 1.45f;
                const juce::Rectangle<float> knobArea (k->centre.x - half, k->centre.y - half,
                                                       half * 2.0f, half * 2.0f);
                const auto r = m.area (knobArea);

                if (r.getX() < 0 || r.getY() < 0 || r.getRight() > s[0] || r.getBottom() > s[1])
                {
                    std::printf ("layout check FAILED: knob %d outside %dx%d (x %d y %d w %d h %d)\n",
                                 i, s[0], s[1], r.getX(), r.getY(), r.getWidth(), r.getHeight());
                    return 1;
                }
            }
        }
    }

    g.saveState();
    g.addTransform (mapping.transform());
    ember::ui::PanelRenderer::renderBackground (g, layout, 1.0f);

    struct Demo { int index; float norm; } ;
    const Demo demo[] = {
        { 0, 0.55f },   // low / hi
        { 1, 0.62f },   // input
        { 2, 0.30f },   // impedance 600
        { 3, 0.56f },   // vernier
        { 4, 0.66f },   // attack
        { 5, 0.60f },   // output
        { 6, 0.50f },   // mix
        { 7, 0.45f }    // release
    };

    for (auto d : demo)
    {
        if (auto* k = layout.knobFor (d.index))
        {
            const float half = k->skirtRadius * 1.45f;
            const auto area = juce::Rectangle<float> (k->centre.x - half, k->centre.y - half, half * 2.0f, half * 2.0f);
            ember::ui::PanelRenderer::paintKnobComponent (g, *k, area, d.norm, false, d.index == 5);
        }
    }

    ember::ui::MeterState meter;
    meter.grDb = 0.0f;
    meter.levelDb = -3.0f;
    meter.grMode = false;
    meter.powered = true;
    ember::ui::PanelRenderer::drawMeterNeedle (g, layout, meter);
    ember::ui::PanelRenderer::drawMeterGlass (g, layout, meter);

    ember::ui::PanelRenderer::drawToggle (g, layout.linkToggle, true);
    ember::ui::PanelRenderer::drawToggle (g, layout.powerToggle, true);

    ember::ui::PanelRenderer::drawLamp (g, layout.lamp, layout.lampRadius, juce::Colour (0xffffa53a), 1.0f);
    ember::ui::PanelRenderer::drawLamp (g, layout.powerLamp, layout.powerLampRadius, juce::Colour (0xffff2d18), 1.0f);

    g.restoreState();

    juce::File out (outPath);
    out.deleteFile();
    juce::FileOutputStream stream (out);
    juce::PNGImageFormat png;
    png.writeImageToStream (image, stream);

    std::printf ("wrote %s (%d x %d)\n", outPath.toRawUTF8(), width, height);
    return 0;
}
