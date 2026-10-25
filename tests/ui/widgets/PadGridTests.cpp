#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/widgets/PadGrid.h"
#include "ui/widgets/PaintHelpers.h"

TEST_CASE("Tab clicks switch the mode and notify", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    PadGrid grid;
    grid.setSize(400, PadGrid::totalHeight);

    PadGrid::Mode notified = PadGrid::Mode::HotCue;
    grid.onModeChanged = [&](PadGrid::Mode mode) { notified = mode; };

    // Children are the four tabs in mode order; Loop is the second.
    dynamic_cast<juce::Button*>(grid.getChildComponent(1))->onClick();
    CHECK(notified == PadGrid::Mode::Loop);
}

TEST_CASE("Empty pads paint in empty", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    PadGrid grid;
    const auto image = paintWidget(grid, 400, PadGrid::totalHeight);

    // Inside the first pad (0..95 × 34..78), clear of its rounded corners.
    CHECK(image.getPixelAt(90, 60).getARGB() == tokens::empty);
}

TEST_CASE("Loop and FX defaults light the selected pad", "[ui][widgets]")
{
    const auto loop = PadGrid::defaultPads(PadGrid::Mode::Loop, 5);
    CHECK(loop[5].background == juce::Colour(tokens::loop));
    CHECK(loop[0].background == juce::Colour(tokens::loopDim));
    CHECK(loop[7].label == "16 beat");

    const auto fx = PadGrid::defaultPads(PadGrid::Mode::Fx, 0);
    CHECK(fx[0].background == juce::Colour(tokens::fx));
    CHECK(fx[1].background == juce::Colour(tokens::fxDim));
    CHECK(fx[7].label == "Roll");
}
