#pragma once

/*
    Design space -> window mapping.

    The panel is authored in a fixed 1200 x 360 space (PanelLayout) and blown up
    uniformly to the window, letterboxed so the proportions never change. The
    editor, the headless preview tool and the CI layout check all go through this
    one struct, because getting this wrong once (a translated-then-scaled
    transform) drew every knob outside the window.
*/

#include "PanelLayout.h"

namespace ember::ui
{
    struct PanelMapping
    {
        float scale = 1.0f;
        juce::Point<float> origin;

        static PanelMapping fit (int width, int height) noexcept
        {
            PanelMapping m;
            const auto w = static_cast<float> (juce::jmax (1, width));
            const auto h = static_cast<float> (juce::jmax (1, height));

            m.scale = juce::jmin (w / kDesignWidth, h / kDesignHeight);
            if (! (m.scale > 0.0f))
                m.scale = 1.0f;

            m.origin = { (w - kDesignWidth * m.scale) * 0.5f,
                         (h - kDesignHeight * m.scale) * 0.5f };
            return m;
        }

        juce::Point<float> toWindow (juce::Point<float> p) const noexcept
        {
            return { origin.x + p.x * scale, origin.y + p.y * scale };
        }

        juce::Rectangle<float> toWindow (juce::Rectangle<float> r) const noexcept
        {
            const auto tl = toWindow (r.getTopLeft());
            return { tl.x, tl.y, r.getWidth() * scale, r.getHeight() * scale };
        }

        /** Window rectangle for a design-space area, rounded outwards so a control
            never loses a pixel of its painted halo. */
        juce::Rectangle<int> area (juce::Rectangle<float> r) const noexcept
        {
            return toWindow (r).toNearestInt ();
        }

        /** The transform that maps design coordinates to window coordinates.
            Note the order: scale first, then translate (JUCE composes
            this-then-that, so `translated(...).scaled(...)` would scale the
            translation too). */
        juce::AffineTransform transform() const noexcept
        {
            return juce::AffineTransform::scale (scale)
                       .translated (origin.x, origin.y);
        }
    };
}
