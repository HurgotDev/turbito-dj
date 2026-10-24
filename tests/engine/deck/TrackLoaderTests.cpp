#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "engine/deck/TrackLoader.h"

TEST_CASE("loadTrack decodes the 124 BPM test WAV", "[engine][loader]")
{
    const juce::File file(TURBITO_SOURCE_DIR "/test-audio/beat_124bpm_30s.wav");

    auto track = loadTrack(file);

    REQUIRE(track != nullptr);
    CHECK(track->audio.getNumChannels() == 2);
    CHECK(track->sampleRate == Catch::Approx(44100.0));
    CHECK(track->name == juce::String("beat_124bpm_30s"));

    const auto seconds = track->audio.getNumSamples() / track->sampleRate;
    CHECK_THAT(seconds, Catch::Matchers::WithinAbs(30.0, 512.0 / 44100.0));
}

TEST_CASE("loadTrack returns nullptr for a missing file", "[engine][loader]")
{
    const juce::File file(TURBITO_SOURCE_DIR "/test-audio/does_not_exist.wav");

    CHECK(loadTrack(file) == nullptr);
}
