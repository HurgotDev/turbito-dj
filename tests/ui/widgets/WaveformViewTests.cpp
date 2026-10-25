#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/widgets/PaintHelpers.h"
#include "ui/widgets/WaveformView.h"

TEST_CASE("WaveformView draws the playhead at the centre", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    WaveformView waveform;
    waveform.setLoop(juce::Range<float>(0.5f, 0.59f));

    const auto image = paintWidget(waveform, 200, tokens::layoutWaveformHeight);

    CHECK(image.getPixelAt(100, 27).getARGB() == tokens::playhead);
}

TEST_CASE("WaveformView draws 0-high bars as no audio", "[ui][widgets]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    WaveformView waveform;
    waveform.setColours(juce::Colour(tokens::deckB), juce::Colour(tokens::waveformPlayedB));
    waveform.setBars({ 0.0f, 40.0f });

    const auto image = paintWidget(waveform, 200, tokens::layoutWaveformHeight);

    CHECK(image.getPixelAt(50, 27).getARGB() == tokens::empty);
    CHECK(image.getPixelAt(150, 27).getARGB() == tokens::deckB);
}
