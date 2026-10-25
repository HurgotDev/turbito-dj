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

    juce::TextButton& textButton(juce::Component& parent, const juce::String& text)
    {
        for (auto* c : parent.getChildren())
            if (auto* button = dynamic_cast<juce::TextButton*>(c); button != nullptr
                                                                   && button->getButtonText() == text)
                return *button;

        FAIL("No button " << text);
        throw std::logic_error("unreachable");
    }

    const juce::File fileA("/music/Luces de Neon.wav");
    const juce::File fileB("/music/Pulso.mp3");

    std::vector<LibraryTrack> twoTracks()
    {
        return { { fileA, "Luces de Neon", "Varela", "124", "8A", "6:12", 1 },
                 { fileB, "Pulso", "Dani Ortega", "128", "2A", "6:30", 2 } };
    }
}

TEST_CASE("Counts read like the mockup", "[ui][library]")
{
    CHECK(formatCount(0) == "0");
    CHECK(formatCount(999) == "999");
    CHECK(formatCount(1284) == "1,284");
    CHECK(formatCount(1234567) == "1,234,567");
}

TEST_CASE("setTracks lists the tracks with their record ids", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    library.setTracks(twoTracks());

    const auto& tracks = library.getTracks();
    REQUIRE(tracks.size() == 2);
    CHECK(tracks[0].file == fileA);
    CHECK(tracks[1].id == 2);
}

TEST_CASE("Toolbar, collections and header report the user's intent", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    library.setBounds(0, 0, 1420, 266);
    library.setTracks(twoTracks());

    juce::String searched;
    int filterIndex = -1, collection = -1, imports = 0, analyses = 0, sortColumn = 0;
    bool ascending = false;
    library.onSearchChanged = [&](const juce::String& text) { searched = text; };
    library.onFilterChanged = [&](int index) { filterIndex = index; };
    library.onCollectionSelected = [&](int index) { collection = index; };
    library.onImport = [&] { ++imports; };
    library.onAnalyze = [&] { ++analyses; };
    library.onSortChanged = [&](int column, bool forwards)
    {
        sortColumn = column;
        ascending = forwards;
    };

    // The editor posts its change asynchronously; run its handler by hand.
    auto& search = child<juce::TextEditor>(library);
    search.setText("pulso", false);
    search.onTextChange();
    CHECK(searched == "pulso");

    child<juce::ComboBox>(library).setSelectedItemIndex(3, juce::sendNotificationSync);
    CHECK(filterIndex == 3);

    textButton(library, "Importar").onClick();
    textButton(library, "Analizar").onClick();
    CHECK(imports == 1);
    CHECK(analyses == 1);

    // Third collection: label row (21), then 4 + 34 per item.
    auto& list = *child<juce::Viewport>(library).getViewedComponent();
    const auto now = juce::Time::getCurrentTime();
    const juce::MouseEvent click(juce::Desktop::getInstance().getMainMouseSource(), { 20.0f, 118.0f },
                                 {}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &list, &list, now,
                                 { 20.0f, 118.0f }, now, 1, false);
    list.mouseUp(click);
    CHECK(collection == 2);
    CHECK(library.getSelectedCollection() == 2);

    // Título to Duración sort on click; Deck and Cargar do not.
    auto& table = child<juce::TableListBox>(library);
    auto& header = table.getHeader();
    header.columnClicked(1, {});
    header.columnClicked(7, {});
    CHECK(header.getSortColumnId() == 0);
    header.columnClicked(LibraryComponent::bpmColumn, {});
    CHECK(header.getSortColumnId() == LibraryComponent::bpmColumn);

    // The header tells the table asynchronously; the table tells the model.
    table.getTableListBoxModel()->sortOrderChanged(LibraryComponent::keyColumn, false);
    CHECK(sortColumn == LibraryComponent::keyColumn);
    CHECK(! ascending);
}

TEST_CASE("Dragging a row carries its file to the decks", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    library.setTracks(twoTracks());
    auto& table = child<juce::TableListBox>(library);

    juce::SparseSet<int> rows;
    rows.addRange({ 1, 2 });
    const auto description = table.getTableListBoxModel()->getDragSourceDescription(rows);
    CHECK(description.toString() == fileB.getFullPathName());
    CHECK(table.getTableListBoxModel()->getDragSourceDescription({}).isVoid());

    using Details = juce::DragAndDropTarget::SourceDetails;
    CHECK(LibraryComponent::draggedFile(Details(description, &table, {})) == fileB);

    juce::Component elsewhere;
    CHECK(LibraryComponent::draggedFile(Details(description, &elsewhere, {})) == juce::File());
    CHECK(LibraryComponent::draggedFile(Details(description, nullptr, {})) == juce::File());
}

TEST_CASE("Row buttons load into a deck and add to Auto Mix", "[ui][library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    LibraryComponent library;
    library.setBounds(0, 0, 1420, 266);
    library.setTracks(twoTracks());

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
    library.setTracks(twoTracks());
    library.setLoadedDeck(1, fileB);

    const auto image = paintWidget(library, 1420, 266);
    const auto table = child<juce::TableListBox>(library).getBounds();
    // Right edge of the Deck column, past the badge; row middles at 30 + 20 and 30 + 61.
    const auto x = table.getX() + 48;
    // Inside the badge's left margin: its letter sits a few px further in, and
    // where the letter lands depends on the platform's font rendering.
    const auto badgeX = table.getX() + 13;

    CHECK(image.getPixelAt(x, table.getY() + 30 + 20) != juce::Colour(tokens::rowActive));
    CHECK(image.getPixelAt(x, table.getY() + 30 + 61) == juce::Colour(tokens::rowActive));
    CHECK(image.getPixelAt(badgeX, table.getY() + 30 + 61) == juce::Colour(tokens::deckB));

    library.setLoadedDeck(1, {});
    const auto cleared = paintWidget(library, 1420, 266);
    CHECK(cleared.getPixelAt(x, table.getY() + 30 + 61) != juce::Colour(tokens::rowActive));
}

TEST_CASE("Library lays out at 1420 x 266", "[ui][library]")
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
