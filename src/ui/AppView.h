#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "core/Action.h"
#include "ui/deck/DeckComponent.h"
#include "ui/deck/WaveformPanel.h"
#include "ui/library/AutoMixPanel.h"
#include "ui/library/LibraryComponent.h"
#include "ui/mixer/MixerComponent.h"
#include "ui/topbar/TopBar.h"

// The whole window as the mockup lays it out: top bar, waveforms, decks and
// mixer at fixed heights, the library with Auto Mix taking the rest. Never
// scrolls. Knows nothing of the engine: deck and mixer input goes out
// through onAction.
class AppView : public juce::Component
{
public:
    static constexpr int deckRowHeight = MixerComponent::preferredHeight;
    static constexpr int mixerWidth = 300;

    AppView();

    TopBar& topBar() noexcept { return top; }
    WaveformPanel& waveforms() noexcept { return waveformPanel; }
    DeckComponent& deck(int index) noexcept { return index == 0 ? deckA : deckB; }
    MixerComponent& mixer() noexcept { return mixerPanel; }
    LibraryComponent& library() noexcept { return libraryPanel; }
    AutoMixPanel& autoMix() noexcept { return autoMixPanel; }

    std::function<void(ActionEvent)> onAction;

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    TopBar top;
    WaveformPanel waveformPanel;
    DeckComponent deckA { 0 }, deckB { 1 };
    MixerComponent mixerPanel;
    LibraryComponent libraryPanel;
    AutoMixPanel autoMixPanel;
};
