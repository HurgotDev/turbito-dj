#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/widgets/JogWheel.h"
#include "ui/widgets/PaintHelpers.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("Jog turns once every 1.8 seconds", "[ui][widgets]")
{
    CHECK_THAT(jogAngleDegrees(0.0), WithinAbs(0.0, 1e-3));
    CHECK_THAT(jogAngleDegrees(0.45), WithinAbs(90.0, 1e-3));
    CHECK_THAT(jogAngleDegrees(1.8), WithinAbs(0.0, 1e-3));
    CHECK_THAT(jogAngleDegrees(2.7), WithinAbs(180.0, 1e-3));
    CHECK_THAT(jogAngleDegrees(-0.45), WithinAbs(270.0, 1e-3));
}

TEST_CASE("Jog times are m:ss with a minus sign for the rest", "[ui][widgets]")
{
    CHECK(formatElapsed(161.4) == "2:41");
    CHECK(formatElapsed(0.0) == "0:00");
    CHECK(formatRemaining(161.0, 372.0) == "-3:31");
    CHECK(formatRemaining(0.0, 0.0) == "-0:00");
}

TEST_CASE("JogWheel paints its ring in empty", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    JogWheel jog;
    jog.setPosition(161.0, 372.0);

    const auto image = paintWidget(jog, 148, 148);

    CHECK(image.getPixelAt(4, 74).getARGB() == tokens::empty);
}
