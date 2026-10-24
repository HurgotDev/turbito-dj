#include "engine/AudioEngine.h"

#include "engine/mixer/Crossfader.h"

AudioEngine::AudioEngine()
{
    for (auto& channel : channels)
        channel.prepare(deviceRate);

    setCrossfader();
}

AudioEngine::~AudioEngine()
{
    LoadRequest queued;

    while (loads.pop(queued))
        delete queued.track;

    collectRetired();
    delete pendingRetire;

    for (auto& deck : decks)
        delete deck.setTrack(nullptr);
}

bool AudioEngine::post(ActionEvent event)
{
    return actions.push(event);
}

bool AudioEngine::load(int deck, std::unique_ptr<Track> track)
{
    if (deck < 0 || deck >= numDecks || ! loads.push({ deck, track.get() }))
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

    for (auto& channel : channels)
        channel.prepare(deviceRate);

    crossfaderA.reset(deviceRate, 0.02);
    crossfaderB.reset(deviceRate, 0.02);
    setCrossfader();
}

void AudioEngine::audioDeviceStopped() {}

void AudioEngine::setCrossfader()
{
    const auto gains = crossfaderGains(crossfaderPosition, crossfaderCurve);
    crossfaderA.setTargetValue(gains.a);
    crossfaderB.setTargetValue(gains.b);
}

void AudioEngine::handle(const ActionEvent& event)
{
    const bool pressed = event.value >= 0.5f;
    const float v = event.value;

    switch (event.action)
    {
        case Action::DeckA_Play:            decks[0].play(pressed); break;
        case Action::DeckA_Cue:             decks[0].cue(pressed); break;
        case Action::DeckB_Play:            decks[1].play(pressed); break;
        case Action::DeckB_Cue:             decks[1].cue(pressed); break;
        case Action::ChannelA_Gain:         channels[0].setGain(v); break;
        case Action::ChannelA_EqHigh:       channels[0].setEqHigh(v); break;
        case Action::ChannelA_EqMid:        channels[0].setEqMid(v); break;
        case Action::ChannelA_EqLow:        channels[0].setEqLow(v); break;
        case Action::ChannelA_Filter:       channels[0].setFilter(v); break;
        case Action::ChannelA_Fader:        channels[0].setFader(v); break;
        case Action::ChannelB_Gain:         channels[1].setGain(v); break;
        case Action::ChannelB_EqHigh:       channels[1].setEqHigh(v); break;
        case Action::ChannelB_EqMid:        channels[1].setEqMid(v); break;
        case Action::ChannelB_EqLow:        channels[1].setEqLow(v); break;
        case Action::ChannelB_Filter:       channels[1].setFilter(v); break;
        case Action::ChannelB_Fader:        channels[1].setFader(v); break;
        case Action::Mixer_Crossfader:      crossfaderPosition = v; setCrossfader(); break;
        case Action::Mixer_CrossfaderCurve: crossfaderCurve = v; setCrossfader(); break;
    }
}

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
        handle(event);

    if (pendingRetire != nullptr && retired.push(pendingRetire))
        pendingRetire = nullptr;

    LoadRequest next;

    if (pendingRetire == nullptr && loads.pop(next))
    {
        pendingRetire = decks[next.deck].setTrack(next.track);

        if (pendingRetire != nullptr && retired.push(pendingRetire))
            pendingRetire = nullptr;
    }

    float peaks[numDecks] {};

    for (int offset = 0; offset < numSamples; offset += maxBlock)
    {
        const int count = juce::jmin(maxBlock, numSamples - offset);

        for (int d = 0; d < numDecks; ++d)
        {
            float* const buffers[2] { scratch[d][0].data(), scratch[d][1].data() };

            for (auto* buffer : buffers)
                juce::FloatVectorOperations::clear(buffer, count);

            decks[d].render(buffers, 2, count, deviceRate);
            channels[d].process(buffers[0], buffers[1], count);
            peaks[d] = juce::jmax(peaks[d], channels[d].peak());
        }

        for (int i = 0; i < count; ++i)
        {
            const float a = crossfaderA.getNextValue();
            const float b = crossfaderB.getNextValue();

            for (int ch = 0; ch < juce::jmin(2, numOutputChannels); ++ch)
                outputChannelData[ch][offset + i] = a * scratch[0][ch][static_cast<size_t>(i)]
                                                  + b * scratch[1][ch][static_cast<size_t>(i)];
        }
    }

    for (int d = 0; d < numDecks; ++d)
    {
        auto& snapshot = state.decks[d];
        snapshot.loaded.store(decks[d].hasTrack());
        snapshot.playing.store(decks[d].isPlaying());
        snapshot.positionSeconds.store(decks[d].positionSeconds());
        snapshot.lengthSeconds.store(decks[d].lengthSeconds());
        snapshot.channelPeak.store(peaks[d]);
    }
}
