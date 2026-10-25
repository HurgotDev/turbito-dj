#pragma once

#include <juce_core/juce_core.h>

#include <vector>

// MP3, WAV, FLAC and AIFF files under folder (recursive, case-insensitive
// extensions, hidden files skipped), sorted by full path.
std::vector<juce::File> findAudioFiles(const juce::File& folder);
