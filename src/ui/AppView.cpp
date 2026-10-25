#include "ui/AppView.h"

#include "ui/look/Tokens.h"

AppView::AppView()
{
    const auto forward = [this](ActionEvent event)
    {
        if (onAction)
            onAction(event);
    };

    deckA.onAction = forward;
    deckB.onAction = forward;
    mixerPanel.onAction = forward;

    const auto load = [this](int deck, const juce::File& file)
    {
        if (libraryPanel.onLoad)
            libraryPanel.onLoad(deck, file);
    };
    deckA.onFileDropped = [load](const juce::File& file) { load(0, file); };
    deckB.onFileDropped = [load](const juce::File& file) { load(1, file); };
    waveformPanel.onFileDropped = load;

    libraryPanel.onAddToAutoMix = [this](const LibraryTrack& track) { autoMixPanel.addTrack(track); };
    autoMixPanel.onCollapsedChanged = [this](bool collapsed) { libraryPanel.setSidePanelCollapsed(collapsed); };
    libraryPanel.setSidePanel(&autoMixPanel);

    for (auto* child : std::initializer_list<juce::Component*> { &top, &waveformPanel, &deckA,
                                                                 &mixerPanel, &deckB, &libraryPanel })
        addAndMakeVisible(child);

    // Clicks on the non-focusable panels land here instead of on the library
    // search, so the app's shortcuts keep working.
    setWantsKeyboardFocus(true);
}

void AppView::resized()
{
    constexpr auto gap = tokens::spacingPanelGap;
    auto area = getLocalBounds().reduced(tokens::spacingWindowPadding);

    top.setBounds(area.removeFromTop(TopBar::preferredHeight));
    area.removeFromTop(gap);
    waveformPanel.setBounds(area.removeFromTop(WaveformPanel::preferredHeight));
    area.removeFromTop(gap);

    auto row = area.removeFromTop(deckRowHeight);
    area.removeFromTop(gap);
    const auto deckWidth = (row.getWidth() - mixerWidth - 2 * gap) / 2;
    deckA.setBounds(row.removeFromLeft(deckWidth));
    deckB.setBounds(row.removeFromRight(deckWidth));
    mixerPanel.setBounds(row.withSizeKeepingCentre(mixerWidth, row.getHeight()));

    libraryPanel.setBounds(area);
}

void AppView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(tokens::bg));
}
