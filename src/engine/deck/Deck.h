#pragma once

#include "core/Track.h"

// Transport and cue for one deck. Positions are in samples of the track.
// Not thread-safe: only the audio thread calls it.
class Deck
{
public:
    // Non-owning. Resets position and cue to 0 and pauses. Returns the old track.
    Track* setTrack(Track* track);

    // Play toggles play/pause on press; CUE is held. Releasing play does nothing.
    void play(bool pressed);
    void cue(bool pressed);

    // Adds the deck output to outputs[0..numChannels). Never allocates.
    void render(float* const* outputs, int numChannels, int numSamples, double deviceRate);

    bool hasTrack() const noexcept { return track != nullptr; }
    bool isPlaying() const noexcept { return playing; }
    double positionSeconds() const noexcept;
    double lengthSeconds() const noexcept;

private:
    void startPlayback();
    void stopPlayback();

    Track* track = nullptr;
    double position = 0.0;
    double cuePoint = 0.0;
    bool playing = false;
    bool cueHeld = false;
    bool previewing = false;  // playback started by holding CUE while paused

    // Click-free fades, in samples of the device rate. fadeSamples is set by
    // render(); the others never reach the logic above.
    int fadeSamples = 0;
    int fadeInDone = 0;          // samples of the main voice since its start
    int tailLeft = 0;            // fade-out tail still rendering while > 0
    double tailPosition = 0.0;   // read position of the tail, from the old stop
};
