#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/library/PanelResizer.h"

namespace
{
    // A drag from x 0 to x dx on the resizer.
    void drag(PanelResizer& resizer, float dx)
    {
        const auto now = juce::Time::getCurrentTime();
        const auto event = [&](float x)
        {
            return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), { x, 0.0f },
                                    {}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &resizer, &resizer, now,
                                    { 0.0f, 0.0f }, now, 1, x != 0.0f);
        };

        resizer.mouseDown(event(0.0f));
        resizer.mouseDrag(event(dx));
        resizer.mouseUp(event(dx));
    }
}

TEST_CASE("PanelResizer drags the width within its range", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    PanelResizer resizer(160, 360, 220);
    auto resizes = 0;
    resizer.onResize = [&] { ++resizes; };

    drag(resizer, 30.0f);
    CHECK(resizer.getPanelWidth() == 250);
    drag(resizer, -500.0f);
    CHECK(resizer.getPanelWidth() == 160);
    drag(resizer, 500.0f);
    CHECK(resizer.getPanelWidth() == 360);
    CHECK(resizes == 3);
}

TEST_CASE("An inverted PanelResizer widens when dragged left", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    PanelResizer resizer(320, 640, 400, true);

    drag(resizer, -40.0f);
    CHECK(resizer.getPanelWidth() == 440);
    drag(resizer, 1000.0f);
    CHECK(resizer.getPanelWidth() == 320);
}
