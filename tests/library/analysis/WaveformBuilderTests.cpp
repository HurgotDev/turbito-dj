#include <catch2/catch_test_macros.hpp>

#include "library/analysis/TrackAnalyzer.h"
#include "library/analysis/WaveformBuilder.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <algorithm>
#include <cmath>
#include <vector>

TEST_CASE("WaveformBuilder emits one peak byte per 50 ms window", "[library][analysis]")
{
    WaveformBuilder builder(1000.0); // 50-sample windows

    std::vector<float> samples(120, 0.0f);
    samples[10] = -0.5f;  // window 0
    samples[60] = 2.0f;   // window 1, clipped to 1
    samples[110] = 0.25f; // partial window 2

    builder.process(samples.data(), 70);
    builder.process(samples.data() + 70, 50);
    const auto peaks = builder.finish();

    REQUIRE(peaks.size() == 3);
    CHECK(peaks[0] == 128);
    CHECK(peaks[1] == 255);
    CHECK(peaks[2] == 64);
}

TEST_CASE("analyzeTrack waveform follows the silences of the test file", "[library][analysis]")
{
    const auto analysis = analyzeTrack(juce::File(TURBITO_SOURCE_DIR "/test-audio/silencios_120bpm.wav"));
    REQUIRE(analysis.has_value());

    const auto& w = analysis->waveform;
    const int total = static_cast<int>(w.size());
    REQUIRE(total > 0);

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(
        formats.createReaderFor(juce::File(TURBITO_SOURCE_DIR "/test-audio/silencios_120bpm.wav")));
    REQUIRE(reader != nullptr);
    const double seconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    CHECK(std::abs(total - seconds * 20.0) <= 1.0);

    // 3.0 s of silence, audio until 23.0 s, silence to the end; one window of margin at edges.
    CHECK(std::all_of(w.begin(), w.begin() + 59, [](auto v) { return v == 0; }));
    // The audio has digital silence between beats, so check each second has sound.
    for (int second = 3; second < 23; ++second)
        CHECK(std::any_of(w.begin() + second * 20, w.begin() + second * 20 + 20, [](auto v) { return v > 0; }));
    CHECK(std::all_of(w.begin() + 461, w.end(), [](auto v) { return v == 0; }));
}
