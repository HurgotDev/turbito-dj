#pragma once

#include <juce_core/juce_core.h>

#include <optional>

struct TrackTags
{
    juce::String title, artist;
    double duration = 0; // seconds
};

// Title and artist from the file's tags (TagLib), duration from the decoder.
// Empty title falls back to the file name; empty artist stays empty.
// nullopt when JUCE cannot open the file as audio. Safe on any thread.
std::optional<TrackTags> readTags(const juce::File& file);
