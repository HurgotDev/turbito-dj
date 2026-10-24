#pragma once

#include "core/Action.h"
#include "core/Track.h"

// Transport and cue for one deck. Positions are in samples of the track.
// Not thread-safe: only the audio thread calls it.
class Deck
{
public:
    // Non-owning. Resets position and cue to 0 and pauses. Returns the old track.
    Track* setTrack(Track* track);

    void handle(const ActionEvent& event);

    // Adds the deck output to outputs[0..numChannels). Never allocates.
    void render(float* const* outputs, int numChannels, int numSamples, double deviceRate);

    bool hasTrack() const noexcept { return track != nullptr; }
    bool isPlaying() const noexcept { return playing; }
    double positionSeconds() const noexcept;
    double lengthSeconds() const noexcept;

private:
    Track* track = nullptr;
    double position = 0.0;
    double cuePoint = 0.0;
    bool playing = false;
    bool cueHeld = false;
    bool previewing = false;  // playback started by holding CUE while paused
};
