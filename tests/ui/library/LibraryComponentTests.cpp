#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/library/LibraryComponent.h"
#include "ui/look/Tokens.h"
#include "ui/widgets/PaintHelpers.h"

namespace
{
    template <typename T>
    T& child(juce::Component& parent)
    {
        for (auto* c : parent.getChildren())
            if (auto* found = dynamic_cast<T*>(c))
                return *found;

        FAIL("No such child");
        throw std::logic_error("unreachable");
    }

    juce::Button& loadButton(juce::TableListBox& table, int row, const juce::String& text)
    {
        auto* cell = table.getCellComponent(7, row);
        REQUIRE(cell != nullptr);

        for (auto* c : cell->getChildren())
            if (auto* button = dynamic_cast<juce::Button*>(c); button->getButtonText() == text)
                return *button;

        FAIL("No button " << text);
        throw std::logic_error("unreachable");
    }

    const juce::File fileA("/music/Luces de Neon.wav");
    const juce::File fileB("/music/Pulso.mp3");
}

TEST_CASE("Counts read like the mockup", "[ui][library]")
{
    CHECK(formatCount(0) == "0");
    CHECK(formatCount(999) == "999");
    CHECK(formatCount(1284) == "1,284");
    CHECK(formatCount(1234567) == "1,234,567");
}

TEST_CASE("addFiles lists new files by name and skips duplicates", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    library.addFiles({ fileA, fileB });
    library.addFiles({ fileB, fileA });

    const auto& tracks = library.getTracks();
    REQUIRE(tracks.size() == 2);
    CHECK(tracks[0].file == fileA);
    CHECK(tracks[0].title == "Luces de Neon");
    CHECK(tracks[1].title == "Pulso");
    CHECK(tracks[1].bpm == juce::String::fromUTF8("—"));
}

TEST_CASE("Row buttons load into a deck and add to Auto Mix", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    library.setBounds(0, 0, 1420, 266);
    library.addFiles({ fileA, fileB });

    std::vector<std::pair<int, juce::File>> loads;
    std::vector<juce::File> added;
    library.onLoad = [&](int deck, const juce::File& file) { loads.emplace_back(deck, file); };
    library.onAddToAutoMix = [&](const LibraryTrack& track) { added.push_back(track.file); };

    auto& table = child<juce::TableListBox>(library);
    // Placed in the Cargar column without waiting for the header's async update.
    CHECK(table.getCellComponent(7, 0)->getX() == table.getHeader().getColumnPosition(6).getX());
    loadButton(table, 0, "A").onClick();
    loadButton(table, 1, "B").onClick();
    loadButton(table, 1, "+").onClick();

    REQUIRE(loads.size() == 2);
    CHECK(loads[0] == std::pair<int, juce::File>(0, fileA));
    CHECK(loads[1] == std::pair<int, juce::File>(1, fileB));
    REQUIRE(added.size() == 1);
    CHECK(added[0] == fileB);
}

TEST_CASE("The loaded track's row is highlighted", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    library.addFiles({ fileA, fileB });
    library.setLoadedDeck(1, fileB);

    const auto image = paintWidget(library, 1420, 266);
    const auto table = child<juce::TableListBox>(library).getBounds();
    // Right edge of the Deck column, past the badge; row middles at 30 + 20 and 30 + 61.
    const auto x = table.getX() + 48;

    CHECK(image.getPixelAt(x, table.getY() + 30 + 20) != juce::Colour(tokens::rowActive));
    CHECK(image.getPixelAt(x, table.getY() + 30 + 61) == juce::Colour(tokens::rowActive));
    CHECK(image.getPixelAt(table.getX() + 20, table.getY() + 30 + 61) == juce::Colour(tokens::deckB));

    library.setLoadedDeck(1, {});
    const auto cleared = paintWidget(library, 1420, 266);
    CHECK(cleared.getPixelAt(x, table.getY() + 30 + 61) != juce::Colour(tokens::rowActive));
}

TEST_CASE("Library lays out at 1420 × 266", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    juce::Component autoMix;
    library.setBounds(0, 0, 1420, 266);

    auto& nav = child<juce::Viewport>(library);
    auto& table = child<juce::TableListBox>(library);
    const auto inside = library.getLocalBounds().reduced(13);

    CHECK(nav.getBounds() == juce::Rectangle<int>(13, 13, 220, 240));
    CHECK(inside.contains(table.getBounds()));
    CHECK(table.getRight() == inside.getRight() - 1);

    library.setSidePanel(&autoMix);
    CHECK(autoMix.getBounds() == juce::Rectangle<int>(inside.getRight() - 400, 13, 400, 240));
    CHECK(table.getRight() < autoMix.getX() - 20);

    library.setSidePanelWidth(1000);
    CHECK(autoMix.getWidth() == 640);

    library.setSidePanel(nullptr);
    CHECK(table.getRight() == inside.getRight() - 1);
}
