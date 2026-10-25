#include "library/analysis/BeatAnalyzer.h"

#include <dsp/onsets/DetectionFunction.h>
#include <dsp/tempotracking/TempoTrackV2.h>

#include <juce_core/juce_core.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr double stepSeconds = 0.01161; // 512 samples at 44.1 kHz
constexpr double maxBinSizeHz = 50.0;   // 1024-sample frame at 44.1 kHz
constexpr double kickCutoffHz = 150.0;

int frameSizeFor(double sampleRate)
{
    int size = 1;
    while (size < sampleRate / maxBinSizeHz)
        size *= 2;
    return size;
}

DFConfig detectionConfig(int stepSize, int frameSize)
{
    DFConfig config;
    config.DFType = DF_COMPLEXSD;
    config.stepSize = stepSize;
    config.frameLength = frameSize;
    config.dbRise = 3;
    config.adaptiveWhitening = false;
    config.whiteningRelaxCoeff = -1;
    config.whiteningFloor = -1;
    return config;
}

double median(std::vector<double> values)
{
    const auto mid = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
    std::nth_element(values.begin(), mid, values.end());
    return *mid;
}
} // namespace

BeatGrid fitBeatGrid(const std::vector<double>& beatSeconds)
{
    if (beatSeconds.size() < 4)
        return {};

    std::vector<double> intervals;
    for (size_t i = 1; i < beatSeconds.size(); ++i)
        intervals.push_back(beatSeconds[i] - beatSeconds[i - 1]);

    double period = median(intervals);
    if (period <= 0.0)
        return {};

    // Grid index of each beat, counted interval by interval so jitter never accumulates
    // (a skipped beat counts as 2).
    std::vector<double> index { 0.0 };
    for (double interval : intervals)
        index.push_back(index.back() + std::max(1.0, std::round(interval / period)));

    // Least squares of beat time against its index refines the frame-quantised median.
    double sumN = 0, sumT = 0, sumNN = 0, sumNT = 0;
    for (size_t i = 0; i < beatSeconds.size(); ++i)
    {
        sumN += index[i];
        sumT += beatSeconds[i];
        sumNN += index[i] * index[i];
        sumNT += index[i] * beatSeconds[i];
    }
    const double count = static_cast<double>(beatSeconds.size());
    const double denominator = count * sumNN - sumN * sumN;
    if (denominator > 0.0)
        period = (count * sumNT - sumN * sumT) / denominator;

    // Fold half/double tempo into the usual DJ range.
    double bpm = 60.0 / period;
    double scale = 1.0;
    while (bpm * scale < 70.0)
        scale *= 2.0;
    while (bpm * scale > 180.0)
        scale /= 2.0;

    BeatGrid grid;
    grid.bpm = std::round(bpm * scale * 100.0) / 100.0;
    const double gridPeriod = 60.0 / grid.bpm;
    period = gridPeriod * scale; // rounded period between the detected beats

    std::vector<double> offsets;
    for (size_t i = 0; i < beatSeconds.size(); ++i)
        offsets.push_back(beatSeconds[i] - index[i] * period);

    grid.firstBeat = std::fmod(median(offsets), gridPeriod);
    if (grid.firstBeat < 0.0)
        grid.firstBeat += gridPeriod;
    if (grid.firstBeat >= gridPeriod) // fmod rounding
        grid.firstBeat = 0.0;
    return grid;
}

BeatAnalyzer::BeatAnalyzer(double rate)
    : sampleRate(rate),
      stepSize(static_cast<int>(rate * stepSeconds)),
      detection(std::make_unique<DetectionFunction>(detectionConfig(stepSize, frameSizeFor(rate)))),
      frames(frameSizeFor(rate), stepSize),
      lowPassCoeff(1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi * kickCutoffHz / rate))
{
}

BeatAnalyzer::~BeatAnalyzer() = default;

void BeatAnalyzer::process(const float* samples, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        lowPass += lowPassCoeff * (samples[i] - lowPass);
        stepEnergy += lowPass * lowPass;
        if (++stepFilled == stepSize)
        {
            lowEnergy.push_back(stepEnergy);
            stepEnergy = 0.0;
            stepFilled = 0;
        }
    }

    frames.process(samples, numSamples,
                   [this](const double* frame) { detectionResults.push_back(detection->processTimeDomain(frame)); });
}

BeatGrid BeatAnalyzer::finish()
{
    frames.finish([this](const double* frame) { detectionResults.push_back(detection->processTimeDomain(frame)); });

    auto end = detectionResults.size();
    while (end > 0 && detectionResults[end - 1] <= 0.0)
        --end;

    // Like Mixxx and the qm-vamp plugin, the first 2 values are skipped (start-up noise).
    if (end <= 2)
        return {};
    std::vector<double> df(detectionResults.begin() + 2, detectionResults.begin() + static_cast<std::ptrdiff_t>(end));
    std::vector<double> beatPeriod(df.size(), 0.0);
    std::vector<double> tempi;

    TempoTrackV2 tracker(static_cast<float>(sampleRate), stepSize);
    tracker.calculateBeatPeriod(df, beatPeriod, tempi);
    std::vector<double> beats;
    tracker.calculateBeats(df, beatPeriod, beats);

    std::vector<double> seconds;
    for (double beat : beats)
        seconds.push_back((beat * stepSize + stepSize / 2) / sampleRate);
    auto grid = fitBeatGrid(seconds);
    if (grid.bpm > 0.0)
        grid.firstBeat = kickPhase(grid);
    return grid;
}

double BeatAnalyzer::kickPhase(const BeatGrid& grid) const
{
    // Bass attack per step: how much the low-band energy rose since the previous step.
    std::vector<double> attack(lowEnergy.size(), 0.0);
    for (size_t k = 1; k < lowEnergy.size(); ++k)
        attack[k] = std::max(0.0, lowEnergy[k] - lowEnergy[k - 1]);

    const double period = 60.0 / grid.bpm;
    const auto attackAt = [&](double phase)
    {
        double sum = 0.0;
        for (double t = phase; t * sampleRate < static_cast<double>(attack.size() * static_cast<size_t>(stepSize)); t += period)
        {
            const auto k = static_cast<size_t>(t * sampleRate / stepSize);
            for (size_t j = k > 0 ? k - 1 : 0; j <= k + 1 && j < attack.size(); ++j)
                sum += attack[j];
        }
        return sum;
    };

    const double offBeat = std::fmod(grid.firstBeat + period / 2.0, period);
    return attackAt(offBeat) > attackAt(grid.firstBeat) ? offBeat : grid.firstBeat;
}
