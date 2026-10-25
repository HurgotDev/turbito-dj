#include "ui/widgets/VuMeter.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include "ui/look/Tokens.h"

namespace
{
    constexpr int segmentCount = 14;
    constexpr int segmentSize = 7;
    constexpr int segmentGap = 2;
    constexpr float segmentRadius = 1.0f;
    constexpr float decayPerTick = 0.85f;  // the falling bar of the phase 3 meter
}

int vuLitSegments(float peakLinear)
{
    int lit = 0;

    for (int k = 0; k < segmentCount; ++k)
        if (peakLinear >= juce::Decibels::decibelsToGain(-3.0f * static_cast<float>(segmentCount - 1 - k)))
            ++lit;

    return lit;
}

VuMeter::VuMeter()
{
    setSize(segmentSize, segmentCount * segmentSize + (segmentCount - 1) * segmentGap);
}

void VuMeter::setPeak(float peakLinear)
{
    shownPeak = juce::jmax(peakLinear, shownPeak * decayPerTick);

    const auto lit = vuLitSegments(shownPeak);

    if (lit != litSegments)
    {
        litSegments = lit;
        repaint();
    }
}

void VuMeter::paint(juce::Graphics& g)
{
    for (int k = 0; k < segmentCount; ++k)
    {
        const auto argb = k >= litSegments ? tokens::empty
                        : k >= 12          ? tokens::vuHigh
                        : k >= 9           ? tokens::vuMid
                                           : tokens::vuLow;

        const auto row = segmentCount - 1 - k;
        g.setColour(juce::Colour(argb));
        g.fillRoundedRectangle(0.0f, static_cast<float>(row * (segmentSize + segmentGap)),
                               static_cast<float>(segmentSize), static_cast<float>(segmentSize),
                               segmentRadius);
    }
}
