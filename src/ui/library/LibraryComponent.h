#pragma once

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/library/PanelResizer.h"

// A count as the mockup shows it: 1284 → "1,284".
juce::String formatCount(int count);

struct LibraryTrack
{
    juce::File file;
    juce::String title, artist, bpm, key, duration;
};

// Library panel: collections, search toolbar, track table and an optional
// side panel (Auto Mix) on the right. Fills the rest of the window's height.
class LibraryComponent : public juce::Component, private juce::TableListBoxModel
{
public:
    struct Collection
    {
        juce::String label, count;
    };

    LibraryComponent();
    ~LibraryComponent() override;

    // Defaults to Todas las pistas, Carpetas, Historial and Placas y drops;
    // the first count follows the number of tracks until this is called.
    void setCollections(std::vector<Collection>);
    void setSelectedCollection(int index);
    int getSelectedCollection() const noexcept { return selectedCollection; }

    void setTracks(std::vector<LibraryTrack>);
    // Title from the file name, "—" elsewhere; files already listed are skipped.
    void addFiles(const juce::Array<juce::File>&);
    const std::vector<LibraryTrack>& getTracks() const noexcept { return tracks; }

    // 0 = deck A, 1 = deck B; an empty file clears it.
    void setLoadedDeck(int deck, const juce::File&);

    // Placed on the right with its own resizer (320–640); nullptr removes it.
    void setSidePanel(juce::Component*);
    void setSidePanelWidth(int width);

    std::function<void(int deck, const juce::File&)> onLoad;
    std::function<void(const LibraryTrack&)> onAddToAutoMix;

    void resized() override;
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;

private:
    class CollectionList;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics&, int row, int width, int height,
                            bool rowIsSelected) override;
    void paintCell(juce::Graphics&, int row, int columnId, int width, int height,
                   bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int row, int columnId, bool isRowSelected,
                                             juce::Component* existing) override;
    void cellDoubleClicked(int row, int columnId, const juce::MouseEvent&) override;

    int loadedDeckOf(int row) const;
    void tracksChanged();
    void importFiles();

    // shortcut: in-memory list filled by Importar, replaced by SQLite in phase 5.
    std::vector<LibraryTrack> tracks;
    std::array<juce::File, 2> loaded;

    std::vector<Collection> collections;
    bool defaultCollections = true;
    int selectedCollection = 0;

    std::unique_ptr<CollectionList> collectionList;
    juce::Viewport collectionView;
    PanelResizer collectionResizer;

    juce::TextEditor search;
    juce::ComboBox filter;
    juce::TextButton analyze, import;
    juce::TableListBox table;

    juce::Component* sidePanel = nullptr;
    PanelResizer sideResizer;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::Rectangle<int> searchBox, filterLabel, tableFrame;
};
