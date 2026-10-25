#pragma once

#include "library/analysis/TrackAnalysis.h"

#include <juce_events/juce_events.h>

#include <functional>
#include <memory>
#include <optional>
#include <set>

// Analyses tracks on a low-priority thread pool (max(1, cpus / 2) threads). Use only
// from the message thread: results arrive there through onResult, and callbacks
// still in flight when the queue is destroyed are dropped.
class AnalysisQueue
{
public:
    AnalysisQueue();
    // Removes queued jobs and waits for running ones, which stop early.
    ~AnalysisQueue();

    // nullopt result = file could not be read. Ignored when trackId is already pending.
    void enqueue(juce::int64 trackId, const juce::File& file);
    int pending() const { return static_cast<int>(pendingIds.size()); }

    std::function<void(juce::int64 trackId, std::optional<TrackAnalysis>)> onResult;

private:
    class Job;

    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
    std::set<juce::int64> pendingIds;
    juce::ThreadPool pool;
};
