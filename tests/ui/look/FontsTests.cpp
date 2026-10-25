#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"

namespace
{
    float width(const juce::Font& font)
    {
        return juce::GlyphArrangement::getStringWidth(font, "Turbito DJ 128.00 BPM");
    }
}

TEST_CASE("Embedded font families load", "[fonts]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    REQUIRE(fonts::uiTypeface() != nullptr);
    REQUIRE(fonts::monoTypeface() != nullptr);
    REQUIRE(fonts::logo(36.0f).getTypefacePtr() != nullptr);

    CHECK(fonts::uiTypeface()->getName() == tokens::fontUi);
    CHECK(fonts::monoTypeface()->getName() == tokens::fontMono);
    CHECK(fonts::logo(36.0f).getTypefacePtr()->getName() == tokens::fontLogo);
}

TEST_CASE("Font weights come from the variable wght axis", "[fonts]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    CHECK(width(fonts::ui(13.0f, 700)) > width(fonts::ui(13.0f, 600)));
    CHECK(width(fonts::ui(13.0f, 600)) > width(fonts::ui(13.0f, 500)));
    CHECK(width(fonts::ui(13.0f, 500)) > width(fonts::ui(13.0f, 400)));

    // Monospaced advances don't change with weight, so compare ink instead.
    const auto ink = [](int weight)
    {
        juce::Image image(juce::Image::SingleChannel, 200, 40, true);
        {
            juce::Graphics g(image);
            g.setColour(juce::Colours::white);
            g.setFont(fonts::mono(22.0f, weight));
            g.drawText("0:00 -5:48", image.getBounds(), juce::Justification::centred);
        }

        double sum = 0.0;
        for (int y = 0; y < image.getHeight(); ++y)
            for (int x = 0; x < image.getWidth(); ++x)
                sum += image.getPixelAt(x, y).getFloatAlpha();
        return sum;
    };
    CHECK_THAT(width(fonts::mono(13.0f, 700)),
               Catch::Matchers::WithinAbs(width(fonts::mono(13.0f, 500)), 0.01));
    CHECK(ink(700) > ink(500) * 1.05);
}

TEST_CASE("Tracking is in em, like CSS letter-spacing", "[fonts]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    const auto font = fonts::ui(11.0f);
    const juce::String text = "ABCDEFGHIJ";
    const auto added = juce::GlyphArrangement::getStringWidth(fonts::tracked(font, 0.08f), text)
                     - juce::GlyphArrangement::getStringWidth(font, text);

    CHECK(added > 0.08f * 11.0f * 9.0f - 0.5f);
    CHECK(added < 0.08f * 11.0f * 10.0f + 0.5f);
}
