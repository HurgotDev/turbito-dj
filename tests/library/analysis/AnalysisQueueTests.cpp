#include <catch2/catch_test_macros.hpp>

#include "library/analysis/AnalysisQueue.h"

#include "MessageLoop.h"

#include <cmath>
#include <map>

TEST_CASE("AnalysisQueue delivers one result per track on the message thread", "[library][analysis]")
{
    const juce::ScopedJuceInitialiser_GUI juce;
    const juce::File dir(TURBITO_SOURCE_DIR "/test-audio");

    std::map<juce::int64, std::optional<TrackAnalysis>> results;
    bool allOnMessageThread = true;

    AnalysisQueue queue;
    queue.onResult = [&](juce::int64 id, std::optional<TrackAnalysis> analysis)
    {
        allOnMessageThread = allOnMessageThread && juce::MessageManager::getInstance()->isThisTheMessageThread();
        results[id] = std::move(analysis);
    };

    queue.enqueue(1, dir.getChildFile("beat_124bpm_30s.wav"));
    queue.enqueue(2, dir.getChildFile("beat_126bpm_30s.wav"));
    queue.enqueue(3, dir.getChildFile("tono_la_menor_8A_120bpm.wav"));
    queue.enqueue(4, dir.getChildFile("silencios_120bpm.wav"));
    queue.enqueue(1, dir.getChildFile("beat_124bpm_30s.wav")); // already pending: skipped
    CHECK(queue.pending() == 4);

    runMessageLoopUntil([&] { return results.size() == 4; }, 60000);

    REQUIRE(results.size() == 4);
    CHECK(allOnMessageThread);
    CHECK(queue.pending() == 0);
    for (const auto& [id, analysis] : results)
    {
        REQUIRE(analysis.has_value());
        CHECK(! analysis->waveform.empty());
    }
    CHECK(std::abs(results[1]->bpm - 124.0) <= 0.1);
    CHECK(std::abs(results[2]->bpm - 126.0) <= 0.1);
    CHECK(results[3]->camelot == juce::String("8A"));
}

TEST_CASE("AnalysisQueue drops results after it is destroyed", "[library][analysis]")
{
    const juce::ScopedJuceInitialiser_GUI juce;
    int calls = 0;

    {
        AnalysisQueue queue;
        queue.onResult = [&](juce::int64, std::optional<TrackAnalysis>) { ++calls; };
        queue.enqueue(1, juce::File(TURBITO_SOURCE_DIR "/test-audio/placa_air_horn.wav"));
        juce::Thread::sleep(500); // the job finishes and posts its callback
    }

    runMessageLoopUntil([] { return false; }, 200);
    CHECK(calls == 0);
}
