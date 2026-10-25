#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/AppView.h"

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

    std::vector<juce::Component*> parts(AppView& view)
    {
        return { &view.topBar(), &view.waveforms(), &view.deck(0), &view.mixer(), &view.deck(1),
                 &view.library() };
    }
}

TEST_CASE("AppView lays out like the mockup at 1440 × 900", "[ui][app]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AppView view;
    view.setBounds(0, 0, 1440, 900);

    CHECK(view.topBar().getBounds() == juce::Rectangle<int>(10, 10, 1420, 52));
    CHECK(view.waveforms().getY() == 72);
    CHECK(view.waveforms().getHeight() == 132);

    const auto deckA = view.deck(0).getBounds();
    const auto mixer = view.mixer().getBounds();
    const auto deckB = view.deck(1).getBounds();
    CHECK(deckA.getX() == 10);
    CHECK(deckA.getY() == 214);
    CHECK(deckA.getHeight() == 400);
    CHECK(mixer.getWidth() == 300);
    CHECK(mixer.getX() - deckA.getRight() == 10);
    CHECK(deckB.getX() - mixer.getRight() == 10);
    CHECK(deckB.getRight() == 1430);
    CHECK(deckA.getWidth() == deckB.getWidth());
    CHECK(mixer.getY() == 214);
    CHECK(deckB.getY() == 214);

    CHECK(view.library().getBounds() == juce::Rectangle<int>(10, 624, 1420, 266));
}

TEST_CASE("AppView fits 1280 × 720 without overlaps", "[ui][app]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AppView view;
    view.setBounds(0, 0, 1280, 720);

    CHECK(view.library().getHeight() == 86);

    const auto all = parts(view);
    for (size_t i = 0; i < all.size(); ++i)
    {
        CHECK(view.getLocalBounds().reduced(10).contains(all[i]->getBounds()));

        for (size_t j = i + 1; j < all.size(); ++j)
            CHECK(! all[i]->getBounds().intersects(all[j]->getBounds()));
    }
}

TEST_CASE("The library's + adds the track to the Auto Mix queue", "[ui][app]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AppView view;
    view.setBounds(0, 0, 1440, 900);
    view.library().addFiles({ juce::File("/music/Pulso.mp3") });

    juce::TableListBox* table = nullptr;
    for (auto* child : view.library().getChildren())
        if (auto* found = dynamic_cast<juce::TableListBox*>(child))
            table = found;

    REQUIRE(table != nullptr);
    auto* cell = table->getCellComponent(7, 0);
    REQUIRE(cell != nullptr);

    const auto before = view.autoMix().getQueue().size();
    findButton(*cell, "+").onClick();

    REQUIRE(view.autoMix().getQueue().size() == before + 1);
    CHECK(view.autoMix().getQueue().back().title == "Pulso");
}

TEST_CASE("Deck input reaches AppView::onAction", "[ui][app]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    AppView view;
    view.setBounds(0, 0, 1440, 900);
    std::vector<ActionEvent> events;
    view.onAction = [&](ActionEvent event) { events.push_back(event); };

    findButton(view.deck(0), "CUE").setState(juce::Button::buttonDown);

    REQUIRE(events.size() == 1);
    CHECK(events[0].action == Action::DeckA_Cue);
    CHECK_THAT(events[0].value, Catch::Matchers::WithinAbs(1.0, 1e-6));
}
