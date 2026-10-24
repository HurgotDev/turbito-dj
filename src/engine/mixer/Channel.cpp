#include "engine/mixer/Channel.h"

#include <cmath>

namespace
{
    constexpr double rampSeconds = 0.02;
    constexpr int filterUpdateInterval = 32;
    constexpr double lowCrossover = 250.0;
    constexpr double highCrossover = 2500.0;
    constexpr double butterworthQ = 0.70710678118654752;
    constexpr float deadZone = 0.02f;

    float eqGain(float v)
    {
        // Kill at 0, unity at 0.5, +6 dB at 1.
        return v <= 0.5f ? (2.0f * v) * (2.0f * v) : juce::Decibels::decibelsToGain(12.0f * (v - 0.5f));
    }
}

void Channel::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;

    const auto lp1 = juce::IIRCoefficients::makeLowPass(sampleRate, lowCrossover, butterworthQ);
    const auto hp1 = juce::IIRCoefficients::makeHighPass(sampleRate, lowCrossover, butterworthQ);
    const auto lp2 = juce::IIRCoefficients::makeLowPass(sampleRate, highCrossover, butterworthQ);
    const auto hp2 = juce::IIRCoefficients::makeHighPass(sampleRate, highCrossover, butterworthQ);
    const auto ap2 = juce::IIRCoefficients::makeAllPass(sampleRate, highCrossover, butterworthQ);

    for (auto& side : sides)
    {
        for (int i = 0; i < 2; ++i)
        {
            side.lowPass1[i].setCoefficients(lp1);
            side.highPass1[i].setCoefficients(hp1);
            side.lowPass2[i].setCoefficients(lp2);
            side.highPass2[i].setCoefficients(hp2);
            side.lowPass1[i].reset();
            side.highPass1[i].reset();
            side.lowPass2[i].reset();
            side.highPass2[i].reset();
        }

        side.allPass2.setCoefficients(ap2);
        side.allPass2.reset();
        side.filter.reset();
    }

    for (auto* smoothed : { &level, &eqLow, &eqMid, &eqHigh, &filterKnob })
        smoothed->reset(sampleRate, rampSeconds);  // also jumps to the target

    filterMode = FilterMode::bypass;
    filterCountdown = 0;
    lastPeak = 0.0f;
}

void Channel::setGain(float normalized)
{
    gainTarget = juce::Decibels::decibelsToGain(juce::jmap(juce::jlimit(0.0f, 1.0f, normalized), -12.0f, 12.0f));
    level.setTargetValue(gainTarget * faderTarget);
}

void Channel::setFader(float normalized)
{
    const float v = juce::jlimit(0.0f, 1.0f, normalized);
    faderTarget = v * v;
    level.setTargetValue(gainTarget * faderTarget);
}

void Channel::setEqHigh(float normalized) { eqHigh.setTargetValue(eqGain(juce::jlimit(0.0f, 1.0f, normalized))); }
void Channel::setEqMid(float normalized) { eqMid.setTargetValue(eqGain(juce::jlimit(0.0f, 1.0f, normalized))); }
void Channel::setEqLow(float normalized) { eqLow.setTargetValue(eqGain(juce::jlimit(0.0f, 1.0f, normalized))); }
void Channel::setFilter(float normalized) { filterKnob.setTargetValue(juce::jlimit(0.0f, 1.0f, normalized)); }

void Channel::updateFilter()
{
    const float v = filterKnob.getCurrentValue();
    const auto mode = v < 0.5f - deadZone ? FilterMode::lowPass
                    : v > 0.5f + deadZone ? FilterMode::highPass
                                          : FilterMode::bypass;

    // A fresh state avoids feeding one filter type's memory into another.
    if (mode != filterMode)
        for (auto& side : sides)
            side.filter.reset();

    filterMode = mode;

    if (mode == FilterMode::bypass)
        return;

    const double edge = 0.5 - deadZone;
    const double maxCutoff = sampleRate * 0.45;
    juce::IIRCoefficients coefficients;

    // Exponential sweeps: LPF 80 Hz (v = 0) to 20 kHz (v = 0.48), HPF 20 Hz
    // (v = 0.52) to 8 kHz (v = 1).
    if (mode == FilterMode::lowPass)
    {
        const double cutoff = 80.0 * std::pow(20000.0 / 80.0, v / edge);
        coefficients = juce::IIRCoefficients::makeLowPass(sampleRate, juce::jmin(cutoff, maxCutoff), 1.0);
    }
    else
    {
        const double cutoff = 20.0 * std::pow(8000.0 / 20.0, (v - (1.0 - edge)) / edge);
        coefficients = juce::IIRCoefficients::makeHighPass(sampleRate, juce::jmin(cutoff, maxCutoff), 1.0);
    }

    // setCoefficients takes a SpinLock, but only this thread ever touches it.
    for (auto& side : sides)
        side.filter.setCoefficients(coefficients);
}

float Channel::processSample(Side& side, float x, float low, float mid, float high)
{
    const float lowBand = side.allPass2.processSingleSampleRaw(
        side.lowPass1[1].processSingleSampleRaw(side.lowPass1[0].processSingleSampleRaw(x)));
    const float upper = side.highPass1[1].processSingleSampleRaw(side.highPass1[0].processSingleSampleRaw(x));
    const float midBand = side.lowPass2[1].processSingleSampleRaw(side.lowPass2[0].processSingleSampleRaw(upper));
    const float highBand = side.highPass2[1].processSingleSampleRaw(side.highPass2[0].processSingleSampleRaw(upper));

    const float y = low * lowBand + mid * midBand + high * highBand;
    return filterMode == FilterMode::bypass ? y : side.filter.processSingleSampleRaw(y);
}

void Channel::process(float* left, float* right, int numSamples)
{
    float blockPeak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        filterKnob.getNextValue();

        if (--filterCountdown <= 0)
        {
            updateFilter();
            filterCountdown = filterUpdateInterval;
        }

        const float low = eqLow.getNextValue();
        const float mid = eqMid.getNextValue();
        const float high = eqHigh.getNextValue();
        const float gain = level.getNextValue();

        left[i] = gain * processSample(sides[0], left[i], low, mid, high);
        right[i] = gain * processSample(sides[1], right[i], low, mid, high);

        blockPeak = juce::jmax(blockPeak, std::abs(left[i]), std::abs(right[i]));
    }

    lastPeak = blockPeak;
}
