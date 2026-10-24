#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

// One stereo mixer channel: gain, 3-band isolator EQ, filter knob and fader.
// Controls take normalized values 0–1. Not thread-safe: only the audio thread
// calls it, except prepare(), which runs before callbacks begin.
class Channel
{
public:
    // Sets coefficients and resets filter state and smoothers. Never call it
    // from the audio callback.
    void prepare(double sampleRate);

    void setGain(float normalized);
    void setEqHigh(float normalized);
    void setEqMid(float normalized);
    void setEqLow(float normalized);
    void setFilter(float normalized);
    void setFader(float normalized);

    // In place. Never allocates.
    void process(float* left, float* right, int numSamples);

    // Absolute peak of the last process() call, post-fader.
    float peak() const noexcept { return lastPeak; }

private:
    enum class FilterMode { bypass, lowPass, highPass };

    // Per stereo side. Each LR4 crossover is two cascaded Butterworth biquads.
    struct Side
    {
        juce::IIRFilter lowPass1[2], highPass1[2];  // 250 Hz crossover
        juce::IIRFilter lowPass2[2], highPass2[2];  // 2.5 kHz crossover
        juce::IIRFilter allPass2;                   // aligns the low band's phase
        juce::IIRFilter filter;                     // filter knob
    };

    float processSample(Side& side, float x, float low, float mid, float high);
    void updateFilter();

    double sampleRate = 44100.0;
    Side sides[2];

    float gainTarget = 1.0f;
    float faderTarget = 1.0f;
    juce::SmoothedValue<float> level { 1.0f };  // gain · fader
    juce::SmoothedValue<float> eqLow { 1.0f }, eqMid { 1.0f }, eqHigh { 1.0f };
    juce::SmoothedValue<float> filterKnob { 0.5f };

    FilterMode filterMode = FilterMode::bypass;
    int filterCountdown = 0;
    float lastPeak = 0.0f;
};
