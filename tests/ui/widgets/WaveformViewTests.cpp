#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
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

TEST_CASE("barsAround centres 8 s of peaks on the playhead", "[ui][widgets]")
{
    std::vector<std::uint8_t> peaks(400, 0);  // 20 s
    peaks[200] = 255;                          // 10 s
    peaks[0] = 51;

    const auto centred = barsAround(peaks, 10.0);
    REQUIRE(centred.size() == 160);
    CHECK_THAT(centred[80], Catch::Matchers::WithinAbs(48.0f, 1e-6));
    CHECK_THAT(centred[79], Catch::Matchers::WithinAbs(0.0f, 1e-6));

    // At the start the first half is before the track: empty.
    const auto start = barsAround(peaks, 0.0);
    CHECK_THAT(start[79], Catch::Matchers::WithinAbs(0.0f, 1e-6));
    CHECK_THAT(start[80], Catch::Matchers::WithinAbs(51.0f / 255.0f * 48.0f, 1e-6));

    // Past the end everything is empty.
    const auto end = barsAround(peaks, 25.0);
    CHECK(end == std::vector<float>(160, 0.0f));

    CHECK(barsAround({}, 3.0, 10) == std::vector<float>(10, 0.0f));
    CHECK(barsAround(peaks, 10.0, 4).size() == 4);
}
