#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <memory>

#include "core/Track.h"

// Sample i holds the value i, so output values show the read position.
inline std::unique_ptr<Track> makeRampTrack(int numSamples, int numChannels, double sampleRate)
{
    auto track = std::make_unique<Track>();
    track->audio.setSize(numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
        for (int i = 0; i < numSamples; ++i)
            track->audio.setSample(ch, i, static_cast<float>(i));

    track->sampleRate = sampleRate;
    track->name = "ramp";
    return track;
}
