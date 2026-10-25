#include "library/analysis/WaveformBuilder.h"

#include <algorithm>
#include <cmath>

WaveformBuilder::WaveformBuilder(double sampleRate)
    : windowSize(std::max(1, static_cast<int>(std::lround(sampleRate / 20.0))))
{
}

void WaveformBuilder::process(const float* samples, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        peak = std::max(peak, std::abs(samples[i]));
        if (++filled == windowSize)
            emit();
    }
}

std::vector<std::uint8_t> WaveformBuilder::finish()
{
    if (filled > 0)
        emit();
    return std::move(peaks);
}

void WaveformBuilder::emit()
{
    peaks.push_back(static_cast<std::uint8_t>(std::lround(std::min(1.0f, peak) * 255.0f)));
    peak = 0.0f;
    filled = 0;
}
