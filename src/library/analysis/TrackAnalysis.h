#pragma once

#include <juce_core/juce_core.h>

#include <cstdint>
#include <vector>

// Result of analysing one file. bpm 0 means not detected.
struct TrackAnalysis
{
    double bpm = 0.0;
    double firstBeat = 0.0;
    juce::String camelot;
    std::vector<std::uint8_t> waveform;
};
