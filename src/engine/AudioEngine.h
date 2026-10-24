#pragma once

#include <juce_audio_devices/juce_audio_devices.h>

#include <memory>

#include "core/Action.h"
#include "core/DeckSnapshot.h"
#include "core/SpscQueue.h"
#include "core/Track.h"
#include "engine/deck/Deck.h"

// Message thread: post, load, collectRetired, snapshot.
// Audio thread: the audioDevice callbacks.
// Remove this callback from the device before destroying the engine.
class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    AudioEngine();
    ~AudioEngine() override;

    // Returns false when the queue is full. The event is dropped.
    bool post(ActionEvent event);

    // Returns false when the queue is full. The track is then freed.
    bool load(std::unique_ptr<Track> track);

    // Deletes tracks the audio thread has replaced. Returns how many.
    int collectRetired();

    const DeckSnapshot& snapshot() const noexcept { return state; }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;

private:
    static constexpr int actionCapacity = 64;
    static constexpr int loadCapacity = 4;
    static constexpr int retiredCapacity = 8;

    SpscQueue<ActionEvent, actionCapacity> actions;
    SpscQueue<Track*, loadCapacity> loads;
    SpscQueue<Track*, retiredCapacity> retired;

    // Audio thread only. Holds a replaced track while the retired queue is full.
    Track* pendingRetire = nullptr;

    // Written in audioDeviceAboutToStart, before callbacks begin.
    double deviceRate = 44100.0;

    Deck deck;
    DeckSnapshot state;
};
