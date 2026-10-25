#pragma once

#include <algorithm>
#include <vector>

// Cuts a mono stream into frames of `size` samples every `step` samples. The first
// frame is centred on sample 0 (half a frame of leading zeros), like Mixxx's
// DownmixAndOverlapHelper, so frame k is centred on sample k * step.
class FrameSplitter
{
public:
    FrameSplitter(int frameSize, int stepSize)
        : buffer(static_cast<size_t>(frameSize), 0.0), step(stepSize), writePos(frameSize / 2)
    {
    }

    template <typename OnFrame>
    void process(const float* samples, int numSamples, OnFrame&& onFrame)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            buffer[static_cast<size_t>(writePos++)] = samples[i];
            if (writePos == static_cast<int>(buffer.size()))
            {
                onFrame(buffer.data());
                std::copy(buffer.begin() + step, buffer.end(), buffer.begin());
                writePos -= step;
            }
        }
    }

    // Pads with zeros so the tail of the stream reaches a frame centre.
    template <typename OnFrame>
    void finish(OnFrame&& onFrame)
    {
        const std::vector<float> zeros(buffer.size() / 2, 0.0f);
        process(zeros.data(), static_cast<int>(zeros.size()), onFrame);
    }

private:
    std::vector<double> buffer;
    int step;
    int writePos;
};
