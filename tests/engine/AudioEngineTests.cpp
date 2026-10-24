#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../TestTracks.h"
#include "engine/AudioEngine.h"

namespace
{
    constexpr double rate = 44100.0;

    juce::AudioBuffer<float> runBlock(AudioEngine& engine, int numSamples)
    {
        juce::AudioBuffer<float> out(2, numSamples);
        out.clear();

        const juce::AudioIODeviceCallbackContext context;
        engine.audioDeviceIOCallbackWithContext(nullptr, 0, out.getArrayOfWritePointers(), 2,
                                                numSamples, context);
        return out;
    }

    // Long enough for the 20 ms smoothers and the EQ filters to settle.
    juce::AudioBuffer<float> settle(AudioEngine& engine)
    {
        for (int i = 0; i < 20; ++i)
            runBlock(engine, 512);

        return runBlock(engine, 512);
    }

    const DeckSnapshot& deckA(const AudioEngine& engine) { return engine.snapshot().decks[0]; }
    const DeckSnapshot& deckB(const AudioEngine& engine) { return engine.snapshot().decks[1]; }
}

TEST_CASE("AudioEngine delivers posted actions to the deck", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(0, makeRampTrack(44100, 2, rate)));
    runBlock(engine, 1);

    CHECK(deckA(engine).loaded.load());
    CHECK_FALSE(deckA(engine).playing.load());

    REQUIRE(engine.post({ Action::DeckA_Play, 1.0f }));
    runBlock(engine, 100);

    CHECK(deckA(engine).playing.load());
    CHECK(deckA(engine).positionSeconds.load() == Catch::Approx(100.0 / rate));
}

TEST_CASE("AudioEngine routes each deck action to its own deck", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(0, makeRampTrack(44100, 2, rate)));
    REQUIRE(engine.load(1, makeRampTrack(44100, 2, rate)));
    runBlock(engine, 1);
    runBlock(engine, 1);

    REQUIRE(engine.post({ Action::DeckB_Play, 1.0f }));
    runBlock(engine, 1);
    CHECK_FALSE(deckA(engine).playing.load());
    CHECK(deckB(engine).playing.load());

    REQUIRE(engine.post({ Action::DeckA_Play, 1.0f }));
    REQUIRE(engine.post({ Action::DeckB_Cue, 1.0f }));
    runBlock(engine, 1);
    CHECK(deckA(engine).playing.load());
    CHECK_FALSE(deckB(engine).playing.load());

    REQUIRE(engine.post({ Action::DeckA_Cue, 1.0f }));
    runBlock(engine, 1);
    CHECK_FALSE(deckA(engine).playing.load());
}

TEST_CASE("AudioEngine retires the replaced track", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(0, makeRampTrack(1000, 2, rate)));
    runBlock(engine, 1);
    CHECK(engine.collectRetired() == 0);

    REQUIRE(engine.load(0, makeRampTrack(2000, 2, rate)));
    runBlock(engine, 1);

    CHECK(deckA(engine).lengthSeconds.load() == Catch::Approx(2000.0 / rate));
    CHECK(engine.collectRetired() == 1);
    CHECK(engine.collectRetired() == 0);
}

TEST_CASE("AudioEngine loading into B retires only B's old track", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(0, makeRampTrack(1000, 2, rate)));
    REQUIRE(engine.load(1, makeRampTrack(3000, 2, rate)));
    runBlock(engine, 1);
    runBlock(engine, 1);
    CHECK(engine.collectRetired() == 0);

    REQUIRE(engine.load(1, makeRampTrack(4000, 2, rate)));
    runBlock(engine, 1);

    CHECK(deckA(engine).lengthSeconds.load() == Catch::Approx(1000.0 / rate));
    CHECK(deckB(engine).lengthSeconds.load() == Catch::Approx(4000.0 / rate));
    CHECK(engine.collectRetired() == 1);
}

TEST_CASE("AudioEngine rejects an invalid deck", "[engine]")
{
    AudioEngine engine;
    CHECK_FALSE(engine.load(2, makeRampTrack(10, 2, rate)));
    CHECK_FALSE(engine.load(-1, makeRampTrack(10, 2, rate)));
}

TEST_CASE("AudioEngine crossfader ends play only one deck", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(0, makeConstantTrack(200000, 0.25f, rate)));
    REQUIRE(engine.load(1, makeConstantTrack(200000, 0.5f, rate)));
    runBlock(engine, 1);
    runBlock(engine, 1);
    REQUIRE(engine.post({ Action::DeckA_Play, 1.0f }));
    REQUIRE(engine.post({ Action::DeckB_Play, 1.0f }));

    REQUIRE(engine.post({ Action::Mixer_Crossfader, 0.0f }));
    auto out = settle(engine);
    CHECK(out.getSample(0, 511) == Catch::Approx(0.25f).margin(1.0e-3f));
    CHECK(out.getSample(1, 511) == Catch::Approx(0.25f).margin(1.0e-3f));

    REQUIRE(engine.post({ Action::Mixer_Crossfader, 1.0f }));
    out = settle(engine);
    CHECK(out.getSample(0, 511) == Catch::Approx(0.5f).margin(1.0e-3f));
    CHECK(out.getSample(1, 511) == Catch::Approx(0.5f).margin(1.0e-3f));
}

TEST_CASE("AudioEngine block larger than maxBlock advances by its size", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(0, makeRampTrack(44100, 2, rate)));
    runBlock(engine, 1);

    REQUIRE(engine.post({ Action::DeckA_Play, 1.0f }));
    runBlock(engine, 3000);

    CHECK(deckA(engine).positionSeconds.load() == Catch::Approx(3000.0 / rate));
}

TEST_CASE("AudioEngine snapshot channel peak follows the fader", "[engine]")
{
    AudioEngine engine;
    engine.audioDeviceAboutToStart(nullptr);
    REQUIRE(engine.load(0, makeConstantTrack(200000, 0.5f, rate)));
    runBlock(engine, 1);
    REQUIRE(engine.post({ Action::DeckA_Play, 1.0f }));

    settle(engine);
    CHECK(deckA(engine).channelPeak.load() == Catch::Approx(0.5f).margin(1.0e-3f));
    CHECK(deckB(engine).channelPeak.load() == Catch::Approx(0.0f).margin(1.0e-9f));

    REQUIRE(engine.post({ Action::ChannelA_Fader, 0.5f }));
    settle(engine);
    CHECK(deckA(engine).channelPeak.load() == Catch::Approx(0.125f).margin(1.0e-3f));
}
