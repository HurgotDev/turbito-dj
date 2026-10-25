#include "library/analysis/AnalysisQueue.h"

#include "library/analysis/TrackAnalyzer.h"

class AnalysisQueue::Job : public juce::ThreadPoolJob
{
public:
    Job(AnalysisQueue& q, juce::int64 id, juce::File f)
        : juce::ThreadPoolJob("analysis"), queue(&q), alive(q.alive), trackId(id), file(std::move(f))
    {
    }

    JobStatus runJob() override
    {
        auto result = analyzeTrack(file, [this] { return shouldExit(); });
        if (shouldExit())
            return jobHasFinished;

        juce::MessageManager::callAsync(
            [q = queue, a = alive, id = trackId, r = std::move(result)]() mutable
            {
                if (! *a)
                    return;
                q->pendingIds.erase(id);
                if (q->onResult)
                    q->onResult(id, std::move(r));
            });
        return jobHasFinished;
    }

private:
    AnalysisQueue* queue;
    std::shared_ptr<bool> alive;
    juce::int64 trackId;
    juce::File file;
};

AnalysisQueue::AnalysisQueue()
    : pool(juce::ThreadPoolOptions {}
               .withThreadName("Analysis")
               .withNumberOfThreads(std::max(1, juce::SystemStats::getNumCpus() / 2))
               .withDesiredThreadPriority(juce::Thread::Priority::low))
{
}

AnalysisQueue::~AnalysisQueue()
{
    *alive = false;
    pool.removeAllJobs(true, 10000);
}

void AnalysisQueue::enqueue(juce::int64 trackId, const juce::File& file)
{
    if (! pendingIds.insert(trackId).second)
        return;
    pool.addJob(new Job(*this, trackId, file), true);
}
