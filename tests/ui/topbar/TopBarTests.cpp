#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/topbar/TopBar.h"
#include "ui/widgets/PaintHelpers.h"

TEST_CASE("Audio status reads like the mockup", "[ui][topbar]")
{
    CHECK(formatAudioStatus(44100.0, 256) == juce::String::fromUTF8("Audio: 44.1 kHz · buffer 256"));
    CHECK(formatAudioStatus(48000.0, 512) == juce::String::fromUTF8("Audio: 48 kHz · buffer 512"));
    CHECK(formatAudioStatus(0.0, 256) == "Audio: sin dispositivo");
}

TEST_CASE("TopBar draws the logo and the Grabar dot", "[ui][topbar]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    TopBar bar;
    const auto image = paintWidget(bar, 1420, TopBar::preferredHeight);

    // Logo box: border 1 + padding 14 across, border 1 + padding 7 down.
    auto logoDrawn = false;
    for (int y = 8; y < 44 && ! logoDrawn; ++y)
        for (int x = 15; x < 15 + 168 && ! logoDrawn; ++x)
            logoDrawn = image.getPixelAt(x, y) != juce::Colour(tokens::panel);
    CHECK(logoDrawn);

    juce::Button* record = nullptr;
    for (auto* child : bar.getChildren())
        if (auto* button = dynamic_cast<juce::Button*>(child);
            button != nullptr && button->getButtonText() == "Grabar")
            record = button;

    REQUIRE(record != nullptr);
    CHECK(record->getHeight() == 34);
    // Dot: 10 box after 12 padding, so its centre is 17 in.
    CHECK(image.getPixelAt(record->getX() + 17, record->getBounds().getCentreY())
          == juce::Colour(tokens::record));
}

TEST_CASE("TopBar buttons call back", "[ui][topbar]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    TopBar bar;
    auto settings = 0;
    auto record = 0;
    bar.onSettings = [&] { ++settings; };
    bar.onRecord = [&] { ++record; };

    for (auto* child : bar.getChildren())
        if (auto* button = dynamic_cast<juce::Button*>(child))
            button->onClick();

    CHECK(settings == 1);
    CHECK(record == 1);
}
