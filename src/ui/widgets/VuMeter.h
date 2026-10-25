#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Number of lit segments (0–14) for a linear peak: segment k lights at
// −3 · (13 − k) dBFS, so the top one needs 0 dBFS.
int vuLitSegments(float peakLinear);

// 14-segment vertical meter, segment 0 at the bottom.
class VuMeter : public juce::Component
{
public:
    VuMeter();

    // Call at UI rate with the latest block peak; the shown level falls
    // between calls.
    void setPeak(float peakLinear);

    void paint(juce::Graphics&) override;

private:
    float shownPeak = 0.0f;
    int litSegments = 0;
};
