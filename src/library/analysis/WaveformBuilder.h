#pragma once

#include <cstdint>
#include <vector>

// Peak of |sample| per 50 ms window (20 per second) as a byte 0-255.
class WaveformBuilder
{
public:
    explicit WaveformBuilder(double sampleRate);

    void process(const float* samples, int numSamples);
    // Flushes the last partial window and returns all peaks.
    std::vector<std::uint8_t> finish();

private:
    void emit();

    int windowSize;
    int filled = 0;
    float peak = 0.0f;
    std::vector<std::uint8_t> peaks;
};
