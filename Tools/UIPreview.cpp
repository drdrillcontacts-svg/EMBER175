/*
    Offline front panel renderer.

    Builds the same graphics that the plug-in editor shows, without needing a
    display, so that the panel design can be reviewed in CI or on a headless
    machine:

        ember_ui_preview out.png [width] [height]

    This target is optional (EMBER_BUILD_UI_PREVIEW).
*/

#include "../Source/UI/PanelRenderer.h"

#include <juce_graphics/juce_graphics.h>
#include <cstdio>
#include <cstdlib>

namespace juce
{
    const char* juce_compilationDate = __DATE__;
    const char* juce_compilationTime = __TIME__;
}

int main (int argc, char** argv)
{
    const juce::String outPath = argc > 1 ? juce::String (argv[1]) : juce::String ("ember175-panel.png");
    const int width = argc > 2 ? std::atoi (argv[2]) : 1800;
    const int height = argc > 3 ? std::atoi (argv[3]) : 900;

    juce::Image image (juce::Image::ARGB, width, height, true);
    juce::Graphics g (image);
    g.fillAll (juce::Colour (0xff171614));

    auto layout = ember::ui::makeLayout();
    const float scale = std::min (static_cast<float> (width) / ember::ui::kDesignWidth,
                                  static_cast<float> (height) / ember::ui::kDesignHeight);
    const float ox = (static_cast<float> (width) - ember::ui::kDesignWidth * scale) * 0.5f;
    const float oy = (static_cast<float> (height) - ember::ui::kDesignHeight * scale) * 0.5f;

    g.saveState();
    g.addTransform (juce::AffineTransform::translation (ox, oy).scaled (scale));
    ember::ui::PanelRenderer::renderBackground (g, layout, 1.0f);

    struct Demo { int index; float norm; };
    const Demo demo[] = {
        { 0, 0.62f },   // input
        { 1, 0.50f },   // output
        { 2, 0.80f },   // attack
        { 3, 0.45f },   // release
        { 4, 0.58f },   // input vernier
        { 5, 0.50f },   // output vernier
        { 6, 0.50f },   // g/r vernier
        { 7, 1.00f },   // mix
        { 8, 0.50f }    // headroom
    };

    for (auto d : demo)
        if (auto* k = layout.knobFor (d.index))
            ember::ui::PanelRenderer::drawKnob (g, *k, d.norm, d.index == 0, false);

    ember::ui::MeterState meter;
    meter.grDb = 4.2f;
    meter.levelDb = -7.0f;
    meter.grMode = true;
    meter.powered = true;
    ember::ui::PanelRenderer::drawMeterNeedle (g, layout, meter);
    ember::ui::PanelRenderer::drawMeterGlass (g, layout, meter);

    ember::ui::PanelRenderer::drawToggle (g, layout.power, true, "ON", true);

    auto link = layout.linkButton;
    link.setHeight (18.0f);
    link.setY (layout.linkButton.getCentreY() - 9.0f);
    ember::ui::PanelRenderer::drawToggle (g, link, true, "LINKED", false);

    auto vern = layout.vernierButton;
    vern.setHeight (18.0f);
    vern.setY (layout.vernierButton.getCentreY() - 9.0f);
    ember::ui::PanelRenderer::drawToggle (g, vern, false, "CONTINUOUS", false);

    auto meterBtn = layout.meterButton;
    meterBtn.setHeight (18.0f);
    meterBtn.setY (layout.meterButton.getCentreY() - 9.0f);
    ember::ui::PanelRenderer::drawToggle (g, meterBtn, true, "GAIN REDUCTION", false);

    ember::ui::PanelRenderer::drawReadout (g, layout.readout, "INPUT  24.0 dB", "GR 4.2 dB   OUT -7.0 dBFS", 1.0f);

    g.restoreState();

    juce::File out (outPath);
    out.deleteFile();
    juce::FileOutputStream stream (out);
    juce::PNGImageFormat png;
    png.writeImageToStream (image, stream);

    std::printf ("wrote %s (%d x %d)\n", outPath.toRawUTF8(), width, height);
    return 0;
}
