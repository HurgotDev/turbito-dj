#pragma once

#include <juce_core/juce_core.h>

#include <memory>

#include "core/Track.h"

// Blocking decode of the whole file. Call it off the audio thread.
std::unique_ptr<Track> loadTrack(const juce::File& file);
