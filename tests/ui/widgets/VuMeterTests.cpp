#include <catch2/catch_test_macros.hpp>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/widgets/PaintHelpers.h"
#include "ui/widgets/VuMeter.h"

namespace
{
    float db(float decibels)
    {
        return juce::Decibels::decibelsToGain(decibels);
    }
}

TEST_CASE("VU segments light from the peak level", "[ui][widgets]")
{
    CHECK(vuLitSegments(0.0f) == 0);
    CHECK(vuLitSegments(1.0f) == 14);
    CHECK(vuLitSegments(db(-3.0f)) == 13);
    CHECK(vuLitSegments(db(-12.0f)) == 10);
    CHECK(vuLitSegments(db(-40.0f)) == 0);
}

TEST_CASE("VuMeter paints the top segment red at full scale", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    VuMeter meter;
    meter.setPeak(1.0f);

    const auto image = paintWidget(meter, 7, 124);

    CHECK(image.getPixelAt(3, 3).getARGB() == tokens::vuHigh);
    CHECK(image.getPixelAt(3, 120).getARGB() == tokens::vuLow);
}
