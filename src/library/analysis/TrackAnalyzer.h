#pragma once

#include "library/analysis/TrackAnalysis.h"

#include <juce_core/juce_core.h>

#include <functional>
#include <optional>

// Reads the file block by block (never whole), downmixes to mono and runs the beat,
// key and waveform analysis in one pass. nullopt when the file cannot be read or
// shouldExit returned true (checked once per block).
std::optional<TrackAnalysis> analyzeTrack(const juce::File& file, const std::function<bool()>& shouldExit = {});
