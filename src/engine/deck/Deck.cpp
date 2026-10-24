#include "engine/deck/Deck.h"

#include <juce_core/juce_core.h>

namespace
{
    constexpr double fadeSeconds = 0.005;

    // Linear interpolation of one channel at a fractional sample position.
    float readInterpolated(const juce::AudioBuffer<float>& audio, int channel, double position)
    {
        const int length = audio.getNumSamples();
        const int index = static_cast<int>(position);
        const int next = juce::jmin(index + 1, length - 1);
        const auto fraction = static_cast<float>(position - index);
        const auto* source = audio.getReadPointer(juce::jmin(channel, audio.getNumChannels() - 1));

        // shortcut: linear interpolation, replace with Rubber Band in phase 6
        return source[index] + (source[next] - source[index]) * fraction;
    }
}

Track* Deck::setTrack(Track* next)
{
    Track* previous = track;
    track = next;
    position = 0.0;
    cuePoint = 0.0;
    playing = false;
    cueHeld = false;
    previewing = false;
    tailLeft = 0;  // the old track may be retired now; never read it again
    return previous;
}

void Deck::play(bool pressed)
{
    if (track == nullptr || ! pressed)
        return;

    if (cueHeld)
    {
        startPlayback();
        previewing = false;
    }
    else if (playing)
    {
        stopPlayback();
    }
    else
    {
        startPlayback();
    }
}

void Deck::cue(bool pressed)
{
    if (track == nullptr)
        return;

    if (pressed)
    {
        cueHeld = true;

        if (playing)
        {
            stopPlayback();
            position = cuePoint;
            previewing = false;
        }
        else
        {
            cuePoint = position;
            startPlayback();
            previewing = true;
        }
    }
    else
    {
        if (cueHeld && previewing)
        {
            stopPlayback();
            position = cuePoint;
        }

        cueHeld = false;
        previewing = false;
    }
}

void Deck::startPlayback()
{
    if (playing)
        return;

    playing = true;
    fadeInDone = 0;
}

void Deck::stopPlayback()
{
    if (! playing)
        return;

    // The tail continues from the current read position, starting at the
    // level the main voice had reached (a stop during fade-in is smooth).
    tailLeft = fadeInDone;
    tailPosition = position;
    playing = false;
}

void Deck::render(float* const* outputs, int numChannels, int numSamples, double deviceRate)
{
    if (track == nullptr || (! playing && tailLeft == 0))
        return;

    const auto& audio = track->audio;
    const int length = audio.getNumSamples();

    if (length == 0 || audio.getNumChannels() == 0)
    {
        playing = false;
        tailLeft = 0;
        return;
    }

    fadeSamples = juce::jmax(1, juce::roundToInt(deviceRate * fadeSeconds));

    if (position >= length)
    {
        position = length;
        playing = false;
    }

    const double step = track->sampleRate / deviceRate;

    for (int i = 0; i < numSamples; ++i)
    {
        if (playing)
        {
            const auto gain = static_cast<float>(fadeInDone) / static_cast<float>(fadeSamples);

            for (int ch = 0; ch < numChannels; ++ch)
                outputs[ch][i] += gain * readInterpolated(audio, ch, position);

            position += step;

            if (fadeInDone < fadeSamples)
                ++fadeInDone;

            if (position >= length)
            {
                position = length;
                playing = false;
            }
        }

        if (tailLeft > 0)
        {
            const auto gain = static_cast<float>(tailLeft) / static_cast<float>(fadeSamples);

            for (int ch = 0; ch < numChannels; ++ch)
                outputs[ch][i] += gain * readInterpolated(audio, ch, tailPosition);

            tailPosition += step;
            --tailLeft;

            if (tailPosition >= length)
                tailLeft = 0;
        }
    }
}

double Deck::positionSeconds() const noexcept
{
    return track == nullptr ? 0.0 : position / track->sampleRate;
}

double Deck::lengthSeconds() const noexcept
{
    return track == nullptr ? 0.0 : track->audio.getNumSamples() / track->sampleRate;
}
