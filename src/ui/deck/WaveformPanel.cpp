#include "ui/deck/WaveformPanel.h"

#include "ui/Strings.h"
#include "ui/library/LibraryComponent.h"
#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    constexpr int padding = 8;
    constexpr int rowGap = 6;
    constexpr int tileWidth = 28;
    constexpr int tileGap = 8;

    constexpr std::uint32_t accents[] = { tokens::deckA, tokens::deckB };
    constexpr std::uint32_t played[] = { tokens::waveformPlayedA, tokens::waveformPlayedB };
}

WaveformPanel::WaveformPanel()
{
    for (int deck = 0; deck < 2; ++deck)
    {
        waveform(deck).setColours(juce::Colour(accents[deck]), juce::Colour(played[deck]));
        addAndMakeVisible(waveform(deck));
    }
}

WaveformView& WaveformPanel::waveform(int deck)
{
    return waveforms[static_cast<std::size_t>(deck)];
}

juce::Rectangle<int> WaveformPanel::tileBounds(int deck) const
{
    return { 1 + padding, 1 + padding + deck * (tokens::layoutWaveformHeight + rowGap), tileWidth,
             tokens::layoutWaveformHeight };
}

void WaveformPanel::resized()
{
    for (int deck = 0; deck < 2; ++deck)
    {
        const auto tile = tileBounds(deck);
        waveform(deck).setBounds(tile.getRight() + tileGap, tile.getY(),
                                 getWidth() - 1 - padding - tile.getRight() - tileGap,
                                 tile.getHeight());
    }
}

void WaveformPanel::paint(juce::Graphics& g)
{
    TurbitoLookAndFeel::drawPanel(g, getLocalBounds().toFloat());

    g.setFont(fonts::ui(static_cast<float>(tokens::fontSizeBody), 700));

    for (int deck = 0; deck < 2; ++deck)
    {
        const auto tile = tileBounds(deck).toFloat();
        g.setColour(juce::Colour(accents[deck]));
        g.fillRoundedRectangle(tile, static_cast<float>(tokens::radiusSmall));
        g.setColour(juce::Colour(tokens::bg));
        g.drawText(strings::deckLetters[deck], tile, juce::Justification::centred, false);
    }
}

bool WaveformPanel::isInterestedInDragSource(const SourceDetails& details)
{
    return LibraryComponent::draggedFile(details) != juce::File();
}

void WaveformPanel::itemDropped(const SourceDetails& details)
{
    if (onFileDropped)
        onFileDropped(details.localPosition.y < tileBounds(1).getY() ? 0 : 1,
                      LibraryComponent::draggedFile(details));
}
