#pragma once

#include <juce_audio_devices/juce_audio_devices.h>

#include <array>
#include <memory>

#include "core/Action.h"
#include "core/EngineSnapshot.h"
#include "core/SpscQueue.h"
#include "core/Track.h"
#include "engine/deck/Deck.h"
#include "engine/mixer/Channel.h"

// Message thread: post, load, collectRetired, snapshot.
// Audio thread: the audioDevice callbacks.
// Remove this callback from the device before destroying the engine.
class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    static constexpr int numDecks = 2;

    AudioEngine();
    ~AudioEngine() override;

    // Returns false when the queue is full. The event is dropped.
    bool post(ActionEvent event);

    // deck: 0 = A, 1 = B. Returns false when the queue is full or the deck is
    // invalid. The track is then freed.
    bool load(int deck, std::unique_ptr<Track> track);

    // Deletes tracks the audio thread has replaced. Returns how many.
    int collectRetired();

    const EngineSnapshot& snapshot() const noexcept { return state; }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;

private:
    struct LoadRequest
    {
        int deck = 0;
        Track* track = nullptr;
    };

    static constexpr int actionCapacity = 64;
    static constexpr int loadCapacity = 4;
    static constexpr int retiredCapacity = 8;

    // Callbacks are processed in chunks of this size, so scratch never grows.
    static constexpr int maxBlock = 1024;

    void handle(const ActionEvent& event);
    void setCrossfader();

    SpscQueue<ActionEvent, actionCapacity> actions;
    SpscQueue<LoadRequest, loadCapacity> loads;
    SpscQueue<Track*, retiredCapacity> retired;

    // Audio thread only. Holds a replaced track while the retired queue is full.
    Track* pendingRetire = nullptr;

    // Written in audioDeviceAboutToStart, before callbacks begin.
    double deviceRate = 44100.0;

    Deck decks[numDecks];
    Channel channels[numDecks];

    // [deck][left, right]
    std::array<float, maxBlock> scratch[numDecks][2] {};

    float crossfaderPosition = 0.5f;
    float crossfaderCurve = 0.0f;
    juce::SmoothedValue<float> crossfaderA, crossfaderB;

    EngineSnapshot state;
};
