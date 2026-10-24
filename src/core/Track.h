#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

struct Track
{
    juce::AudioBuffer<float> audio;
    double sampleRate = 0.0;
    juce::String name;
};
