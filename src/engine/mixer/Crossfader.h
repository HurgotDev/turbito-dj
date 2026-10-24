#pragma once

#include <juce_core/juce_core.h>

#include <cmath>

struct CrossfaderGains
{
    float a;
    float b;
};

// position 0 = only A, 1 = only B. curve 0 = constant power, 1 = sharp cut
// (s = 16^curve makes each side reach full gain s times sooner).
inline CrossfaderGains crossfaderGains(float position, float curve)
{
    const float x = juce::jlimit(0.0f, 1.0f, position);
    const float s = std::pow(16.0f, juce::jlimit(0.0f, 1.0f, curve));
    const float halfPi = juce::MathConstants<float>::halfPi;

    return { std::sin(halfPi * juce::jmin(1.0f, (1.0f - x) * s)),
             std::sin(halfPi * juce::jmin(1.0f, x * s)) };
}
