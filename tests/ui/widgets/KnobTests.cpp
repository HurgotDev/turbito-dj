#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/widgets/Knob.h"
#include "ui/widgets/PaintHelpers.h"

TEST_CASE("Knob sweeps 270 degrees around 12 o'clock", "[ui][widgets]")
{
    using Catch::Matchers::WithinAbs;

    CHECK_THAT(knobAngleDegrees(0.0f), WithinAbs(-135.0, 1e-4));
    CHECK_THAT(knobAngleDegrees(0.5f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(knobAngleDegrees(1.0f), WithinAbs(135.0, 1e-4));
}

TEST_CASE("Knob paints its marker in the accent", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    Knob knob(26, 0.5);
    knob.setAccent(juce::Colour(tokens::deckA));

    const auto image = paintWidget(knob, 26, 26);

    CHECK(image.getPixelAt(13, 5).getARGB() == tokens::deckA);
}
