#include "engine/deck/Deck.h"

#include <juce_core/juce_core.h>

Track* Deck::setTrack(Track* next)
{
    Track* previous = track;
    track = next;
    position = 0.0;
    cuePoint = 0.0;
    playing = false;
    cueHeld = false;
    previewing = false;
    return previous;
}

void Deck::handle(const ActionEvent& event)
{
    if (track == nullptr)
        return;

    const bool pressed = event.value >= 0.5f;

    if (event.action == Action::DeckA_Play)
    {
        if (! pressed)
            return;

        if (cueHeld)
        {
            playing = true;
            previewing = false;
        }
        else
        {
            playing = ! playing;
        }
    }
    else if (pressed)
    {
        cueHeld = true;

        if (playing)
        {
            position = cuePoint;
            playing = false;
            previewing = false;
        }
        else
        {
            cuePoint = position;
            playing = true;
            previewing = true;
        }
    }
    else
    {
        if (cueHeld && previewing)
        {
            position = cuePoint;
            playing = false;
        }

        cueHeld = false;
        previewing = false;
    }
}

void Deck::render(float* const* outputs, int numChannels, int numSamples, double deviceRate)
{
    if (track == nullptr || ! playing)
        return;

    const auto& audio = track->audio;
    const int length = audio.getNumSamples();

    if (length == 0 || audio.getNumChannels() == 0)
    {
        playing = false;
        return;
    }

    if (position >= length)
    {
        position = length;
        playing = false;
        return;
    }

    const double step = track->sampleRate / deviceRate;

    for (int i = 0; i < numSamples && playing; ++i)
    {
        const int index = static_cast<int>(position);
        const int next = juce::jmin(index + 1, length - 1);
        const auto fraction = static_cast<float>(position - index);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const auto* source = audio.getReadPointer(juce::jmin(ch, audio.getNumChannels() - 1));

            // shortcut: linear interpolation, replace with Rubber Band in phase 6
            outputs[ch][i] += source[index] + (source[next] - source[index]) * fraction;
        }

        position += step;

        if (position >= length)
        {
            position = length;
            playing = false;
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
