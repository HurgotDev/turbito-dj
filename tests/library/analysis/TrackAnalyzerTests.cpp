#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "library/analysis/BeatAnalyzer.h"
#include "library/analysis/TrackAnalyzer.h"

#include <algorithm>
#include <cmath>

using Catch::Matchers::WithinAbs;

namespace
{
juce::File testAudio(const char* name)
{
    return juce::File(TURBITO_SOURCE_DIR "/test-audio").getChildFile(name);
}

// Distance from firstBeat to the expected phase on a circular grid of period 60 / bpm.
double phaseError(double firstBeat, double expected, double bpm)
{
    const double period = 60.0 / bpm;
    const double d = std::fmod(std::abs(firstBeat - expected), period);
    return std::min(d, period - d);
}
} // namespace

TEST_CASE("analyzeTrack finds 124 BPM and the first beat at 0 s", "[library][analysis]")
{
    const auto analysis = analyzeTrack(testAudio("beat_124bpm_30s.wav"));
    REQUIRE(analysis.has_value());

    CHECK_THAT(analysis->bpm, WithinAbs(124.0, 0.1));
    CHECK(phaseError(analysis->firstBeat, 0.0, analysis->bpm) <= 0.02);
    CHECK(analysis->firstBeat >= 0.0);
    CHECK(analysis->firstBeat < 60.0 / analysis->bpm);
}

TEST_CASE("analyzeTrack finds 126 BPM", "[library][analysis]")
{
    const auto analysis = analyzeTrack(testAudio("beat_126bpm_30s.wav"));
    REQUIRE(analysis.has_value());

    CHECK_THAT(analysis->bpm, WithinAbs(126.0, 0.1));
}

TEST_CASE("analyzeTrack finds A minor (8A) at 120 BPM", "[library][analysis]")
{
    const auto analysis = analyzeTrack(testAudio("tono_la_menor_8A_120bpm.wav"));
    REQUIRE(analysis.has_value());

    CHECK(analysis->camelot == juce::String("8A"));
    CHECK_THAT(analysis->bpm, WithinAbs(120.0, 0.1));
}

TEST_CASE("analyzeTrack returns nullopt for unreadable files", "[library][analysis]")
{
    CHECK_FALSE(analyzeTrack(testAudio("does_not_exist.wav")).has_value());
    CHECK_FALSE(analyzeTrack(testAudio("README.md")).has_value());
}

TEST_CASE("analyzeTrack stops when asked to exit", "[library][analysis]")
{
    CHECK_FALSE(analyzeTrack(testAudio("beat_124bpm_30s.wav"), [] { return true; }).has_value());
}

TEST_CASE("fitBeatGrid fits a constant grid to jittered beats", "[library][analysis]")
{
    std::vector<double> beats;
    const double period = 60.0 / 128.0;
    for (int i = 0; i < 40; ++i)
        beats.push_back(1.1 + i * period + ((i % 2) ? 0.005 : -0.005));

    const auto grid = fitBeatGrid(beats);
    CHECK_THAT(grid.bpm, WithinAbs(128.0, 0.01));
    CHECK(phaseError(grid.firstBeat, 1.1, 128.0) <= 0.005);
    CHECK(grid.firstBeat < period);
}

TEST_CASE("fitBeatGrid folds tempos into 70-180 BPM", "[library][analysis]")
{
    std::vector<double> slow, fast;
    for (int i = 0; i < 20; ++i)
    {
        slow.push_back(i * 60.0 / 62.0);  // 62 BPM -> 124
        fast.push_back(i * 60.0 / 250.0); // 250 BPM -> 125
    }

    CHECK_THAT(fitBeatGrid(slow).bpm, WithinAbs(124.0, 0.01));
    CHECK_THAT(fitBeatGrid(fast).bpm, WithinAbs(125.0, 0.01));
}

TEST_CASE("fitBeatGrid needs at least 4 beats", "[library][analysis]")
{
    CHECK_THAT(fitBeatGrid({ 0.0, 0.5, 1.0 }).bpm, WithinAbs(0.0, 1e-9));
}
