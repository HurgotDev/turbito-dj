#include "ui/library/LibraryComponent.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "ui/Layout.h"
#include "ui/Strings.h"
#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    constexpr int inset = 1 + tokens::spacingPanelPadding;  // border + padding
    constexpr int resizerMargin = 4;
    constexpr int collapsedMargin = 8;
    constexpr int columnGap = 10;
    constexpr int toolbarHeight = 38;  // the search box: 36 + its border (content-box)
    constexpr int controlHeight = 36;
    constexpr int toolbarGap = 8;
    constexpr int filterGap = 6;
    constexpr int controlPadding = 10;
    constexpr int searchIconSize = 14;
    constexpr int buttonPadding = 12;
    constexpr int comboExtra = 8 + 16;  // text padding + chevron area
    constexpr int frameRadius = tokens::radiusButton;

    constexpr int itemHeight = 34;
    constexpr int itemMinHeight = 16;  // one line of Inter 13
    constexpr int itemGap = 4;
    constexpr int itemPadding = 10;
    constexpr int labelPaddingX = 8;
    constexpr int labelPaddingY = 4;

    constexpr int headerHeight = 30;
    constexpr int rowHeight = 41;  // 1px top border + 6 + 28 + 6
    constexpr int cellPadding = 10;
    constexpr int badgeSize = 22;
    constexpr int loadSize = 28;
    constexpr int loadGap = 6;

    // Título to Duración are LibraryComponent::Column.
    enum { deckColumn = 1, loadColumn = 7 };
    // Measured in mockup-1440x900.png; Título and Artista share the rest.
    constexpr int fixedWidths[] = { 56, 0, 0, 50, 59, 91, 120 };

    constexpr std::uint32_t accents[] = { tokens::deckA, tokens::deckB };

    juce::Font labelFont()
    {
        return fonts::tracked(fonts::ui(static_cast<float>(tokens::fontSizeLabel)),
                              tokens::labelTracking);
    }

    juce::Font itemFont() { return fonts::ui(static_cast<float>(tokens::fontSizeBody)); }
    juce::Font countFont() { return fonts::ui(static_cast<float>(tokens::fontSizeLabel)); }
    juce::Font smallFont() { return fonts::ui(static_cast<float>(tokens::fontSizeButton)); }
    juce::Font buttonFont() { return fonts::ui(static_cast<float>(tokens::fontSizeButton), 600); }

    int labelRowHeight()
    {
        return 2 * labelPaddingY + juce::roundToInt(labelFont().getHeight());
    }

    int buttonWidth(const juce::TextButton& button)
    {
        return 2 * buttonPadding + 2 + textWidth(buttonFont(), button.getButtonText());
    }

    juce::Path searchIcon()
    {
        juce::Path path;
        path.addEllipse(4.0f, 4.0f, 14.0f, 14.0f);
        path.startNewSubPath(20.0f, 20.0f);
        path.lineTo(16.5f, 16.5f);
        return path;
    }

    // The A, B and + buttons of a row's Cargar cell.
    struct LoadButtons : juce::Component
    {
        std::array<juce::TextButton, 3> buttons;
        std::function<void(int button)> onClick;

        LoadButtons()
        {
            const std::uint32_t borders[] = { tokens::deckA, tokens::deckB, tokens::borderControl };
            const char* texts[] = { strings::deckLetters[0], strings::deckLetters[1],
                                    strings::addToAutoMix };

            for (std::size_t i = 0; i < buttons.size(); ++i)
            {
                auto& button = buttons[i];
                button.setButtonText(texts[i]);
                button.setColour(TurbitoLookAndFeel::buttonBorderColourId, juce::Colour(borders[i]));
                button.getProperties().set(TurbitoLookAndFeel::radiusProperty, tokens::radiusSmall);
                TurbitoLookAndFeel::setButtonFont(button, i == 2 ? 15 : tokens::fontSizeLabel, 700);
                button.setWantsKeyboardFocus(false);
                button.onClick = [this, i] { if (onClick) onClick(static_cast<int>(i)); };
                addAndMakeVisible(button);
            }
        }

        void resized() override
        {
            auto x = cellPadding;

            for (auto& button : buttons)
            {
                button.setBounds(x, 1 + (getHeight() - 1 - loadSize) / 2, loadSize, loadSize);
                x += loadSize + loadGap;
            }
        }
    };
}

juce::String formatCount(int count)
{
    auto digits = juce::String(std::abs(count));

    for (auto i = digits.length() - 3; i > 0; i -= 3)
        digits = digits.substring(0, i) + "," + digits.substring(i);

    return (count < 0 ? "-" : "") + digits;
}

//==============================================================================
class LibraryComponent::CollectionList : public juce::Component
{
public:
    explicit CollectionList(LibraryComponent& libraryToUse) : library(libraryToUse) {}

    // Like the mockup's flex column: items shrink towards one line before
    // the list scrolls.
    // Fractional, as the browser shrinks them; each item rounds its own edges.
    float itemHeight = static_cast<float>(::itemHeight);

    int preferredHeight() const
    {
        return labelRowHeight()
             + static_cast<int>(std::ceil(static_cast<float>(library.collections.size())
                                          * (itemGap + itemHeight)));
    }

    juce::Rectangle<int> itemBounds(int index) const
    {
        const auto top = static_cast<float>(labelRowHeight() + itemGap)
                       + static_cast<float>(index) * (itemHeight + itemGap);
        const auto y = juce::roundToInt(top);
        return { 0, y, getWidth(), juce::roundToInt(top + itemHeight) - y };
    }

    void paint(juce::Graphics& g) override
    {
        TurbitoLookAndFeel::drawLabelText(g, strings::library,
                                          juce::Rectangle<int>(getWidth(), labelRowHeight())
                                              .reduced(labelPaddingX, labelPaddingY)
                                              .toFloat(),
                                          juce::Justification::centredLeft);

        for (int i = 0; i < static_cast<int>(library.collections.size()); ++i)
        {
            const auto& collection = library.collections[static_cast<std::size_t>(i)];
            const auto item = itemBounds(i);

            if (i == library.selectedCollection)
            {
                g.setColour(juce::Colour(tokens::empty));
                g.fillRoundedRectangle(item.toFloat(), static_cast<float>(tokens::radiusSmall));
            }

            auto content = item.reduced(itemPadding, 0);
            g.setColour(juce::Colour(tokens::textMuted));
            g.setFont(countFont());
            g.drawText(collection.count, content, juce::Justification::centredRight, false);

            if (collection.count.isNotEmpty())
                content.removeFromRight(textWidth(countFont(), collection.count) + 8);

            g.setColour(juce::Colour(tokens::text));
            g.setFont(itemFont());
            g.drawText(collection.label, content, juce::Justification::centredLeft, true);
        }
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        for (int i = 0; i < static_cast<int>(library.collections.size()); ++i)
            if (itemBounds(i).contains(event.getPosition()))
            {
                library.setSelectedCollection(i);

                if (library.onCollectionSelected)
                    library.onCollectionSelected(i);
            }
    }

private:
    LibraryComponent& library;
};

//==============================================================================
LibraryComponent::LibraryComponent()
    : collectionList(std::make_unique<CollectionList>(*this)),
      collectionResizer(tokens::layoutCollectionsWidthMin, tokens::layoutCollectionsWidthMax,
                        tokens::layoutCollectionsWidthDefault),
      table({}, this),
      sideResizer(tokens::layoutAutoMixWidthMin, tokens::layoutAutoMixWidthMax,
                  tokens::layoutAutoMixWidthDefault, true)
{
    for (const auto* label : strings::collections)
        collections.push_back({ label, {} });

    collectionView.setViewedComponent(collectionList.get(), false);
    // Hidden like macOS overlay scrollbars; the wheel still scrolls.
    collectionView.setScrollBarsShown(false, false, true, false);
    addAndMakeVisible(collectionView);

    for (auto* resizer : { &collectionResizer, &sideResizer })
        resizer->onResize = [this] { resized(); };

    addAndMakeVisible(collectionResizer);
    addChildComponent(sideResizer);

    const auto transparent = juce::Colour(tokens::bg).withAlpha(0.0f);
    search.setFont(itemFont());
    // Closest token to Chrome's placeholder grey (#757575).
    search.setTextToShowWhenEmpty(juce::String::fromUTF8(strings::searchPlaceholder),
                                  juce::Colour(tokens::switchOff));
    search.setJustification(juce::Justification::centredLeft);
    search.setIndents(0, 0);
    search.setBorder({});
    search.setColour(juce::TextEditor::backgroundColourId, transparent);
    search.setColour(juce::TextEditor::outlineColourId, transparent);
    search.setColour(juce::TextEditor::focusedOutlineColourId, transparent);
    // Escape, or a click elsewhere in the library, focuses the library
    // instead of the search, so key presses reach the app's shortcuts again.
    search.onEscapeKey = [this] { grabKeyboardFocus(); };
    search.onTextChange = [this] { if (onSearchChanged) onSearchChanged(search.getText()); };
    setWantsKeyboardFocus(true);
    addAndMakeVisible(search);

    for (int i = 0; i < 4; ++i)
        filter.addItem(juce::String::fromUTF8(strings::libraryFilters[i]), i + 1);
    filter.setSelectedId(1, juce::dontSendNotification);
    filter.setWantsKeyboardFocus(false);
    filter.onChange = [this] { if (onFilterChanged) onFilterChanged(filter.getSelectedItemIndex()); };
    addAndMakeVisible(filter);

    analyze.setButtonText(strings::analyze);
    import.setButtonText(strings::importTracks);
    analyze.onClick = [this] { if (onAnalyze) onAnalyze(); };
    import.onClick = [this] { if (onImport) onImport(); };

    for (auto* button : { &analyze, &import })
    {
        button->setWantsKeyboardFocus(false);
        addAndMakeVisible(button);
    }

    auto& header = table.getHeader();
    for (int i = 0; i < 7; ++i)
        header.addColumn(juce::String::fromUTF8(strings::trackColumns[i]), i + 1,
                         juce::jmax(1, fixedWidths[i]), 1, -1,
                         juce::TableHeaderComponent::visible
                             | (i + 1 == deckColumn || i + 1 == loadColumn
                                    ? 0
                                    : juce::TableHeaderComponent::sortable));
    header.setPopupMenuActive(false);
    table.setHeaderHeight(headerHeight);
    table.setRowHeight(rowHeight);
    table.setWantsKeyboardFocus(false);
    table.setColour(juce::ListBox::outlineColourId, transparent);
    addAndMakeVisible(table);

    tracksChanged();
}

LibraryComponent::~LibraryComponent()
{
    collectionView.setViewedComponent(nullptr, false);
}

void LibraryComponent::setCollections(std::vector<Collection> newCollections)
{
    collections = std::move(newCollections);
    defaultCollections = false;
    selectedCollection = juce::jlimit(0, juce::jmax(0, static_cast<int>(collections.size()) - 1),
                                      selectedCollection);
    resized();
    collectionList->repaint();
}

void LibraryComponent::setSelectedCollection(int index)
{
    selectedCollection = index;
    collectionList->repaint();
}

void LibraryComponent::setTracks(std::vector<LibraryTrack> newTracks)
{
    tracks = std::move(newTracks);
    tracksChanged();
}

void LibraryComponent::setLoadedDeck(int deck, const juce::File& file)
{
    loaded[static_cast<std::size_t>(deck)] = file;
    table.repaint();
}

void LibraryComponent::setSidePanel(juce::Component* panel)
{
    if (sidePanel != nullptr)
        removeChildComponent(sidePanel);

    sidePanel = panel;

    if (sidePanel != nullptr)
        addAndMakeVisible(sidePanel);

    sideResizer.setVisible(sidePanel != nullptr && ! sideCollapsed);
    resized();
}

void LibraryComponent::setSidePanelCollapsed(bool collapsed)
{
    sideCollapsed = collapsed;
    sideResizer.setVisible(sidePanel != nullptr && ! sideCollapsed);
    resized();
}

void LibraryComponent::setSidePanelWidth(int width)
{
    sideResizer.setPanelWidth(width);
    resized();
}

void LibraryComponent::tracksChanged()
{
    if (defaultCollections && ! collections.empty())
        collections.front().count = formatCount(static_cast<int>(tracks.size()));

    table.updateContent();
    resized();
    repaint();
}

//==============================================================================
void LibraryComponent::resized()
{
    auto area = getLocalBounds().reduced(inset);

    // Collections, then their resizer.
    const auto navWidth = collectionResizer.getPanelWidth();
    const auto nav = area.removeFromLeft(navWidth);
    collectionView.setBounds(nav);
    const auto items = juce::jmax(1, static_cast<int>(collections.size()));
    collectionList->itemHeight = juce::jlimit(
        static_cast<float>(itemMinHeight), static_cast<float>(itemHeight),
        static_cast<float>(nav.getHeight() - labelRowHeight() - items * itemGap)
            / static_cast<float>(items));
    collectionList->setSize(navWidth, collectionList->preferredHeight());

    area.removeFromLeft(resizerMargin);
    collectionResizer.setBounds(area.removeFromLeft(PanelResizer::preferredWidth));
    area.removeFromLeft(resizerMargin);

    // Side panel and its resizer from the right.
    if (sidePanel != nullptr && sideCollapsed)
    {
        sidePanel->setBounds(area.removeFromRight(tokens::layoutAutoMixWidthCollapsed));
        area.removeFromRight(collapsedMargin);
    }
    else if (sidePanel != nullptr)
    {
        sidePanel->setBounds(area.removeFromRight(sideResizer.getPanelWidth()));
        area.removeFromRight(resizerMargin);
        sideResizer.setBounds(area.removeFromRight(PanelResizer::preferredWidth));
        area.removeFromRight(resizerMargin);
    }

    // Toolbar: search | Filtro + select | Analizar | Importar.
    auto toolbar = area.removeFromTop(toolbarHeight);
    area.removeFromTop(columnGap);

    const auto control = [&toolbar](int width)
    {
        return toolbar.removeFromRight(width).withSizeKeepingCentre(width, controlHeight);
    };

    import.setBounds(control(buttonWidth(import)));
    toolbar.removeFromRight(toolbarGap);
    analyze.setBounds(control(buttonWidth(analyze)));
    toolbar.removeFromRight(toolbarGap);

    auto comboWidth = 0;
    for (const auto* text : strings::libraryFilters)
        comboWidth = juce::jmax(comboWidth, textWidth(smallFont(), juce::String::fromUTF8(text)));
    filter.setBounds(control(comboWidth + comboExtra));
    toolbar.removeFromRight(filterGap);
    filterLabel = toolbar.removeFromRight(textWidth(smallFont(), strings::filter));
    toolbar.removeFromRight(toolbarGap);

    searchBox = toolbar;
    search.setBounds(toolbar.reduced(1 + controlPadding, 1)
                         .withTrimmedLeft(searchIconSize + toolbarGap));

    // Table: Deck, BPM, Tono, Duración and Cargar fixed; Título and Artista
    // share the rest a little in favour of Título, as the browser does.
    tableFrame = area;
    table.setBounds(area.reduced(1));

    auto rest = table.getVisibleRowWidth();
    for (auto width : fixedWidths)
        rest -= width;

    auto& header = table.getHeader();
    const auto titleWidth = juce::jmax(40, rest * 52 / 100);
    for (int id = deckColumn; id <= loadColumn; ++id)
        header.setColumnWidth(id, id == titleColumn  ? titleWidth
                                  : id == artistColumn ? juce::jmax(40, rest - titleWidth)
                                                       : fixedWidths[id - 1]);

    // Header notifies asynchronously; place the Cargar buttons now.
    table.tableColumnsResized(&header);
}

void LibraryComponent::paint(juce::Graphics& g)
{
    TurbitoLookAndFeel::drawPanel(g, getLocalBounds().toFloat());

    const auto box = searchBox.toFloat().reduced(0.5f);
    const auto radius = static_cast<float>(tokens::radiusButton);
    g.setColour(juce::Colour(tokens::bg));
    g.fillRoundedRectangle(box, radius);
    g.setColour(juce::Colour(tokens::borderControl));
    g.drawRoundedRectangle(box, radius, 1.0f);

    const auto icon = juce::Rectangle<float>(static_cast<float>(searchIconSize),
                                             static_cast<float>(searchIconSize))
                          .withPosition(static_cast<float>(searchBox.getX() + 1 + controlPadding),
                                        static_cast<float>(searchBox.getCentreY()) - searchIconSize / 2.0f);
    static const auto magnifier = searchIcon();
    g.setColour(juce::Colour(tokens::textMuted));
    // JUCE strokes after transforming, so scale the viewBox's 2px stroke by hand.
    g.strokePath(magnifier, juce::PathStrokeType(2.0f * icon.getWidth() / 24.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded),
                 juce::AffineTransform::scale(icon.getWidth() / 24.0f)
                     .translated(icon.getX(), icon.getY()));

    g.setFont(smallFont());
    g.drawText(strings::filter, filterLabel, juce::Justification::centredLeft, false);
}

void LibraryComponent::paintOverChildren(juce::Graphics& g)
{
    // The table frame clips its square header corners, like overflow: auto
    // with border-radius in the mockup.
    const auto frame = tableFrame.toFloat();
    const auto radius = static_cast<float>(frameRadius);

    juce::Path corners;
    corners.addRectangle(frame);
    corners.addRoundedRectangle(frame, radius);
    corners.setUsingNonZeroWinding(false);
    g.setColour(juce::Colour(tokens::panel));
    g.fillPath(corners);

    g.setColour(juce::Colour(tokens::border));
    g.drawRoundedRectangle(frame.reduced(0.5f), radius - 0.5f, 1.0f);
}

//==============================================================================
int LibraryComponent::getNumRows()
{
    return static_cast<int>(tracks.size());
}

int LibraryComponent::loadedDeckOf(int row) const
{
    if (row < 0 || row >= static_cast<int>(tracks.size()))
        return -1;

    const auto& file = tracks[static_cast<std::size_t>(row)].file;

    for (std::size_t deck = 0; deck < loaded.size(); ++deck)
        if (loaded[deck] != juce::File() && loaded[deck] == file)
            return static_cast<int>(deck);

    return -1;
}

void LibraryComponent::paintRowBackground(juce::Graphics& g, int row, int width, int height, bool)
{
    if (loadedDeckOf(row) >= 0)
        g.fillAll(juce::Colour(tokens::rowActive));

    g.setColour(juce::Colour(tokens::empty));
    g.fillRect(0, 0, width, 1);
    juce::ignoreUnused(height);
}

void LibraryComponent::paintCell(juce::Graphics& g, int row, int columnId, int width, int height,
                                 bool)
{
    if (row < 0 || row >= static_cast<int>(tracks.size()))
        return;

    const auto& track = tracks[static_cast<std::size_t>(row)];
    const auto cell = juce::Rectangle<int>(width, height).withTrimmedTop(1).reduced(cellPadding, 0);

    if (columnId == deckColumn)
    {
        const auto deck = loadedDeckOf(row);

        if (deck < 0)
            return;

        const auto badge = juce::Rectangle<int>(badgeSize, badgeSize)
                               .withPosition(cell.getX(), cell.getCentreY() - badgeSize / 2);
        g.setColour(juce::Colour(accents[deck]));
        g.fillRoundedRectangle(badge.toFloat(), 5.0f);
        g.setColour(juce::Colour(tokens::bg));
        g.setFont(fonts::ui(static_cast<float>(tokens::fontSizeLabel), 700));
        g.drawText(strings::deckLetters[deck], badge, juce::Justification::centred, false);
        return;
    }

    const auto body = static_cast<float>(tokens::fontSizeBody);
    juce::String text;

    switch (columnId)
    {
        case titleColumn:    text = track.title;    g.setFont(fonts::ui(body, 500)); break;
        case artistColumn:   text = track.artist;   g.setFont(fonts::ui(body)); break;
        case bpmColumn:      text = track.bpm;      g.setFont(fonts::mono(body)); break;
        case keyColumn:      text = track.key;      g.setFont(fonts::mono(body)); break;
        case durationColumn: text = track.duration; g.setFont(fonts::mono(body)); break;
        default: return;
    }

    g.setColour(juce::Colour(columnId == artistColumn ? tokens::textSoft : tokens::text));
    g.drawText(text, cell, juce::Justification::centredLeft, true);
}

juce::Component* LibraryComponent::refreshComponentForCell(int row, int columnId, bool,
                                                           juce::Component* existing)
{
    if (columnId != loadColumn)
    {
        delete existing;
        return nullptr;
    }

    auto* buttons = dynamic_cast<LoadButtons*>(existing);

    if (buttons == nullptr)
    {
        delete existing;
        buttons = new LoadButtons();
    }

    buttons->onClick = [this, row](int button)
    {
        if (row >= static_cast<int>(tracks.size()))
            return;

        const auto& track = tracks[static_cast<std::size_t>(row)];

        if (button < 2)
        {
            if (onLoad)
                onLoad(button, track.file);
        }
        else if (onAddToAutoMix)
        {
            onAddToAutoMix(track);
        }
    };

    return buttons;
}

void LibraryComponent::cellDoubleClicked(int row, int, const juce::MouseEvent&)
{
    if (onLoad && row >= 0 && row < static_cast<int>(tracks.size()))
        onLoad(0, tracks[static_cast<std::size_t>(row)].file);
}

void LibraryComponent::sortOrderChanged(int columnId, bool ascending)
{
    if (onSortChanged)
        onSortChanged(columnId, ascending);
}

juce::var LibraryComponent::getDragSourceDescription(const juce::SparseSet<int>& rows)
{
    if (rows.isEmpty() || rows[0] < 0 || rows[0] >= static_cast<int>(tracks.size()))
        return {};

    return tracks[static_cast<std::size_t>(rows[0])].file.getFullPathName();
}

juce::File LibraryComponent::draggedFile(const juce::DragAndDropTarget::SourceDetails& details)
{
    const auto* source = details.sourceComponent.get();
    const auto fromLibrary = source != nullptr
                          && dynamic_cast<const juce::TableListBox*>(source) != nullptr
                          && dynamic_cast<const LibraryComponent*>(source->getParentComponent()) != nullptr;

    if (! fromLibrary || ! details.description.isString())
        return {};

    return juce::File(details.description.toString());
}
