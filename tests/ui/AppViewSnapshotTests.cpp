#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "MockupFixture.h"
#include "ui/AppView.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    constexpr float scale = 2.0f;  // the mockup PNG's device pixel ratio

    // The pixel at a point in 1× window coordinates.
    juce::Colour at(const juce::Image& image, int x, int y)
    {
        return image.getPixelAt(juce::roundToInt(static_cast<float>(x) * scale),
                                juce::roundToInt(static_cast<float>(y) * scale));
    }

    const juce::Button* findButton(const juce::Component& parent, const juce::String& text)
    {
        for (auto* child : parent.getChildren())
        {
            if (auto* button = dynamic_cast<const juce::Button*>(child))
                if (button->getButtonText() == text)
                    return button;

            if (auto* found = findButton(*child, text))
                return found;
        }

        return nullptr;
    }
}

TEST_CASE("AppView renders the mockup state at 1440 x 900 @2x", "[ui][app][snapshot]")
{
    const juce::ScopedJuceInitialiser_GUI juce;
    TurbitoLookAndFeel lookAndFeel;
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);

    juce::Image image;
    juce::Point<int> recordDot;
    {
        AppView view;
        view.setBounds(0, 0, 1440, 900);
        mockup::apply(view);
        image = view.createComponentSnapshot(view.getLocalBounds(), true, scale);

        // Grabar's dot: its button's left border and padding, then half the dot.
        // Its width depends on the label's font metrics, so it is not a constant.
        const auto* record = findButton(view.topBar(), "Grabar");
        REQUIRE(record != nullptr);
        recordDot = view.topBar().getPosition() + record->getPosition()
                  + juce::Point<int>(1 + 12 + 5, record->getHeight() / 2);
    }

    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

    REQUIRE(image.getWidth() == 2880);
    REQUIRE(image.getHeight() == 1800);

    const auto file = juce::File(TURBITO_BINARY_DIR).getChildFile("screenshots/app-1440x900.png");
    REQUIRE(file.getParentDirectory().createDirectory());
    file.deleteFile();
    {
        juce::FileOutputStream stream(file);
        REQUIRE(stream.openedOk());
        REQUIRE(juce::PNGImageFormat().writeImageToStream(image, stream));
    }

    const auto is = [](std::uint32_t argb) { return juce::Colour(argb); };

    CHECK(at(image, 4, 4) == is(tokens::bg));
    CHECK(at(image, 700, 14) == is(tokens::panel));      // top bar, above its content
    CHECK(at(image, 30, 240) == is(tokens::deckA));      // deck A letter tile
    CHECK(at(image, 360, 300) == is(tokens::deckA));     // PAUSA, lit while playing
    CHECK(at(image, recordDot.x, recordDot.y) == is(tokens::record));  // Grabar's dot
    CHECK(at(image, 738, 108) == is(tokens::playhead));  // playhead A
    CHECK(at(image, 738, 168) == is(tokens::playhead));  // playhead B
    CHECK(at(image, 1100, 354) == is(tokens::deckB));    // deck B SYNC, lit
    CHECK(at(image, 200, 505) == is(tokens::hotCues[1]));  // hot cue 2, Verso
}
