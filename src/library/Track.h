#pragma once

#include <juce_core/juce_core.h>

#include <optional>

// One row of the library, without the waveform (read it apart, on load).
struct LibraryRecord
{
    juce::int64 id = 0;
    juce::File file;
    juce::String title;
    juce::String artist;
    double duration = 0.0;
    std::optional<double> bpm;
    std::optional<double> firstBeat;
    juce::String camelot;
    bool analyzed = false;
    juce::int64 folderId = 0;
};
