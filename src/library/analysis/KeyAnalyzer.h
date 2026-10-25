#pragma once

#include "library/analysis/FrameSplitter.h"

#include <juce_core/juce_core.h>

#include <array>
#include <memory>

class GetKeyMode;

// qm-dsp GetKeyMode per frame; the track key is the most frequent one, in Camelot.
class KeyAnalyzer
{
public:
    explicit KeyAnalyzer(double sampleRate);
    ~KeyAnalyzer();

    void process(const float* samples, int numSamples);
    // "" when no key was detected.
    juce::String finish();

private:
    void processFrame(const double* frame);

    std::unique_ptr<GetKeyMode> keyMode;
    FrameSplitter frames;
    std::array<int, 25> histogram {};
};
