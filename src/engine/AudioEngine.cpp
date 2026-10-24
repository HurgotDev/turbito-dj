#include "engine/AudioEngine.h"

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine()
{
    Track* queued = nullptr;

    while (loads.pop(queued))
        delete queued;

    collectRetired();
    delete pendingRetire;
    delete deck.setTrack(nullptr);
}

bool AudioEngine::post(ActionEvent event)
{
    return actions.push(event);
}

bool AudioEngine::load(std::unique_ptr<Track> track)
{
    if (! loads.push(track.get()))
        return false;

    track.release();
    return true;
}

int AudioEngine::collectRetired()
{
    int count = 0;
    Track* old = nullptr;

    while (retired.pop(old))
    {
        delete old;
        ++count;
    }

    return count;
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device != nullptr)
        deviceRate = device->getCurrentSampleRate();
}

void AudioEngine::audioDeviceStopped() {}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const*,
                                                   int,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext&)
{
    for (int ch = 0; ch < numOutputChannels; ++ch)
        juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);

    ActionEvent event {};

    while (actions.pop(event))
        deck.handle(event);

    if (pendingRetire != nullptr && retired.push(pendingRetire))
        pendingRetire = nullptr;

    Track* next = nullptr;

    if (pendingRetire == nullptr && loads.pop(next))
    {
        pendingRetire = deck.setTrack(next);

        if (pendingRetire != nullptr && retired.push(pendingRetire))
            pendingRetire = nullptr;
    }

    deck.render(outputChannelData, numOutputChannels, numSamples, deviceRate);

    state.loaded.store(deck.hasTrack());
    state.playing.store(deck.isPlaying());
    state.positionSeconds.store(deck.positionSeconds());
    state.lengthSeconds.store(deck.lengthSeconds());
}
