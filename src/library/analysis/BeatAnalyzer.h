#pragma once

#include "library/analysis/FrameSplitter.h"

#include <memory>
#include <vector>

class DetectionFunction;

struct BeatGrid
{
    double bpm = 0.0; // 0 = not detected
    double firstBeat = 0.0;
};

// Constant grid through beat times in seconds: tempo folded into 70-180 BPM and
// rounded to 0.01, firstBeat in [0, 60 / bpm). Fewer than 4 beats -> bpm 0.
BeatGrid fitBeatGrid(const std::vector<double>& beatSeconds);

// qm-dsp onset detection + TempoTrackV2, set up like Mixxx's AnalyzerQueenMaryBeats.
class BeatAnalyzer
{
public:
    explicit BeatAnalyzer(double sampleRate);
    ~BeatAnalyzer();

    void process(const float* samples, int numSamples);
    BeatGrid finish();

private:
    // TempoTrackV2 may lock onto off-beat hats: keep the grid phase (or the half-beat
    // shift) where the bass attacks are strongest, since kicks mark the beat.
    double kickPhase(const BeatGrid& grid) const;

    double sampleRate;
    int stepSize;
    std::unique_ptr<DetectionFunction> detection;
    FrameSplitter frames;
    std::vector<double> detectionResults;

    double lowPassCoeff;
    double lowPass = 0.0;
    double stepEnergy = 0.0;
    int stepFilled = 0;
    std::vector<double> lowEnergy; // per DF step
};
