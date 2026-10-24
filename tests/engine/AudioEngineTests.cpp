#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../TestTracks.h"
#include "engine/AudioEngine.h"

namespace
{
    void runBlock(AudioEngine& engine, int numSamples)
    {
        juce::AudioBuffer<float> out(2, numSamples);
        out.clear();

        const juce::AudioIODeviceCallbackContext context;
        engine.audioDeviceIOCallbackWithContext(nullptr, 0, out.getArrayOfWritePointers(), 2,
                                                numSamples, context);
    }
}

TEST_CASE("AudioEngine delivers posted actions to the deck", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(makeRampTrack(44100, 2, 44100.0)));
    runBlock(engine, 1);

    CHECK(engine.snapshot().loaded.load());
    CHECK_FALSE(engine.snapshot().playing.load());

    REQUIRE(engine.post({ Action::DeckA_Play, 1.0f }));
    runBlock(engine, 100);

    CHECK(engine.snapshot().playing.load());
    CHECK(engine.snapshot().positionSeconds.load() == Catch::Approx(100.0 / 44100.0));
}

TEST_CASE("AudioEngine retires the replaced track", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(makeRampTrack(1000, 2, 44100.0)));
    runBlock(engine, 1);
    CHECK(engine.collectRetired() == 0);

    REQUIRE(engine.load(makeRampTrack(2000, 2, 44100.0)));
    runBlock(engine, 1);

    CHECK(engine.snapshot().lengthSeconds.load() == Catch::Approx(2000.0 / 44100.0));
    CHECK(engine.collectRetired() == 1);
    CHECK(engine.collectRetired() == 0);
}
