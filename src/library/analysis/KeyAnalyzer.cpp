#include "library/analysis/KeyAnalyzer.h"

#include "library/analysis/Camelot.h"

#include <dsp/keydetection/GetKeyMode.h>

#include <algorithm>

namespace
{
std::unique_ptr<GetKeyMode> makeKeyMode(double sampleRate)
{
    GetKeyMode::Config config(sampleRate, 440.0f);
    config.frameOverlapFactor = 8;
    return std::make_unique<GetKeyMode>(config);
}
} // namespace

KeyAnalyzer::KeyAnalyzer(double sampleRate)
    : keyMode(makeKeyMode(sampleRate)), frames(keyMode->getBlockSize(), keyMode->getHopSize())
{
}

KeyAnalyzer::~KeyAnalyzer() = default;

void KeyAnalyzer::process(const float* samples, int numSamples)
{
    frames.process(samples, numSamples, [this](const double* frame) { processFrame(frame); });
}

juce::String KeyAnalyzer::finish()
{
    frames.finish([this](const double* frame) { processFrame(frame); });

    const auto best = std::max_element(histogram.begin() + 1, histogram.end());
    return *best == 0 ? juce::String() : toCamelot(static_cast<int>(best - histogram.begin()));
}

void KeyAnalyzer::processFrame(const double* frame)
{
    // process() takes a non-const pointer but only reads the frame.
    const int key = keyMode->process(const_cast<double*>(frame));
    if (key >= 0 && key < static_cast<int>(histogram.size()))
        ++histogram[static_cast<size_t>(key)];
}
