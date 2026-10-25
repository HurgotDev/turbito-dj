#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/deck/DeckComponent.h"
#include "ui/look/Tokens.h"
#include "ui/widgets/PaintHelpers.h"

namespace
{
    juce::Button& findButton(juce::Component& parent, const juce::String& text)
    {
        for (auto* child : parent.getChildren())
            if (auto* button = dynamic_cast<juce::Button*>(child);
                button != nullptr && button->getButtonText() == text)
                return *button;

        FAIL("No button " << text);
        throw std::logic_error("unreachable");
    }

    struct Recorder
    {
        std::vector<ActionEvent> events;

        explicit Recorder(DeckComponent& deck)
        {
            deck.onAction = [this](ActionEvent event) { events.push_back(event); };
        }
    };
}

TEST_CASE("Pitch reads like the mockup", "[ui][deck]")
{
    CHECK(formatPitch(0.0f) == "0.0%");
    CHECK(formatPitch(1.2f) == "+1.2%");
    CHECK(formatPitch(-1.6f) == "-1.6%");
    CHECK(formatPitch(1e-15f) == "0.0%");
    CHECK(formatPitch(-0.01f) == "0.0%");
}

TEST_CASE("CUE sends 1 while held and 0 on release", "[ui][deck]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    DeckComponent deck(0);
    Recorder recorder(deck);
    auto& cue = findButton(deck, "CUE");

    cue.setState(juce::Button::buttonDown);
    cue.setState(juce::Button::buttonOver);

    REQUIRE(recorder.events.size() == 2);
    CHECK(recorder.events[0].action == Action::DeckA_Cue);
    CHECK_THAT(recorder.events[0].value, Catch::Matchers::WithinAbs(1.0, 1e-6));
    CHECK(recorder.events[1].action == Action::DeckA_Cue);
    CHECK_THAT(recorder.events[1].value, Catch::Matchers::WithinAbs(0.0, 1e-6));
}

TEST_CASE("PLAY sends a toggle press for its deck", "[ui][deck]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    DeckComponent deckA(0);
    DeckComponent deckB(1);
    Recorder a(deckA);
    Recorder b(deckB);

    findButton(deckA, "PLAY").onClick();
    deckB.setPlaying(true);
    findButton(deckB, "PAUSA").onClick();
    findButton(deckB, "CUE").setState(juce::Button::buttonDown);

    REQUIRE(a.events.size() == 1);
    CHECK(a.events[0].action == Action::DeckA_Play);
    CHECK_THAT(a.events[0].value, Catch::Matchers::WithinAbs(1.0, 1e-6));

    REQUIRE(b.events.size() == 2);
    CHECK(b.events[0].action == Action::DeckB_Play);
    CHECK(b.events[1].action == Action::DeckB_Cue);
}

TEST_CASE("Deck keeps the 148 jog at 550 x 400", "[ui][deck]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    DeckComponent deck(0);
    deck.setBounds(0, 0, 550, 400);

    for (auto* child : deck.getChildren())
    {
        if (dynamic_cast<JogWheel*>(child) != nullptr)
        {
            CHECK(child->getWidth() == tokens::layoutJogSize);
            CHECK(child->getHeight() == tokens::layoutJogSize);
        }

        CHECK(deck.getLocalBounds().contains(child->getBounds()));
    }
}

TEST_CASE("Deck paints its letter tile and a lit PLAY in the accent", "[ui][deck]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    DeckComponent deck(0);
    deck.setPlaying(true);
    const auto image = paintWidget(deck, 550, 400);

    // Left edge of the 36 tile at (13, 14), clear of corners and the letter.
    CHECK(image.getPixelAt(16, 32).getARGB() == tokens::deckA);

    const auto play = findButton(deck, "PAUSA").getBounds();
    CHECK(image.getPixelAt(play.getX() + 10, play.getCentreY()).getARGB() == tokens::deckA);
}
