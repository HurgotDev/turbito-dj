#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <vector>

#include "../../TestTracks.h"
#include "engine/deck/Deck.h"

namespace
{
    constexpr double fileRate = 44100.0;

    juce::AudioBuffer<float> render(Deck& deck, int numSamples, double deviceRate = fileRate)
    {
        juce::AudioBuffer<float> out(2, numSamples);
        out.clear();
        deck.render(out.getArrayOfWritePointers(), 2, numSamples, deviceRate);
        return out;
    }

    enum class Button { Play, Cue };

    void set(Deck& deck, Button button, bool pressed)
    {
        if (button == Button::Play)
            deck.play(pressed);
        else
            deck.cue(pressed);
    }

    void press(Deck& deck, Button button) { set(deck, button, true); }
    void release(Deck& deck, Button button) { set(deck, button, false); }

    bool isSilent(const juce::AudioBuffer<float>& buffer)
    {
        return buffer.getMagnitude(0, buffer.getNumSamples()) < 1.0e-9f;
    }

    double positionSamples(const Deck& deck)
    {
        return deck.positionSeconds() * fileRate;
    }

    // Fade length at 44.1 kHz: 5 ms, as Deck computes it.
    const int fadeSamples = juce::roundToInt(fileRate * 0.005);
}

TEST_CASE("Deck without a track stays silent and paused", "[engine][deck]")
{
    Deck deck;

    press(deck, Button::Play);

    CHECK_FALSE(deck.isPlaying());
    CHECK(isSilent(render(deck, 64)));
}

TEST_CASE("Paused deck renders silence and keeps its position", "[engine][deck]")
{
    auto track = makeRampTrack(1000, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    CHECK(isSilent(render(deck, 100)));
    CHECK(positionSamples(deck) == Catch::Approx(0.0).margin(1.0e-9));
}

TEST_CASE("Play advances the position by numSamples", "[engine][deck]")
{
    auto track = makeRampTrack(1000, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    const auto out = render(deck, 300);

    CHECK(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(300.0));
    CHECK(out.getSample(0, 299) == Catch::Approx(299.0f));
}

TEST_CASE("Play from silence fades in over 5 ms", "[engine][deck]")
{
    auto track = makeConstantTrack(10000, 0.5f, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    const auto out = render(deck, 1000);

    CHECK(out.getSample(0, 0) == Catch::Approx(0.0f).margin(1.0e-6f));
    CHECK(out.getSample(0, fadeSamples / 2) == Catch::Approx(0.25f).margin(1.0e-3f));
    CHECK(out.getSample(0, fadeSamples) == Catch::Approx(0.5f));
    CHECK(out.getSample(0, 999) == Catch::Approx(0.5f));
}

TEST_CASE("Play while playing pauses with a 5 ms fade-out", "[engine][deck]")
{
    auto track = makeRampTrack(1000, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    render(deck, 50);
    press(deck, Button::Play);

    CHECK_FALSE(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(50.0));

    render(deck, fadeSamples);
    CHECK(isSilent(render(deck, 50)));
    CHECK(positionSamples(deck) == Catch::Approx(50.0));
}

TEST_CASE("Pause fades out to exactly zero within 5 ms", "[engine][deck]")
{
    auto track = makeConstantTrack(10000, 0.5f, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    render(deck, 1000);
    press(deck, Button::Play);

    const auto out = render(deck, 1000);

    CHECK(out.getSample(0, 0) == Catch::Approx(0.5f));
    CHECK(out.getSample(0, fadeSamples - 1) == Catch::Approx(0.5f / static_cast<float>(fadeSamples)));
    CHECK(out.getSample(0, fadeSamples) == Catch::Approx(0.0f).margin(0.0f));
    CHECK(out.getSample(0, 999) == Catch::Approx(0.0f).margin(0.0f));
}

TEST_CASE("setTrack cancels a fade-out tail", "[engine][deck]")
{
    auto first = makeConstantTrack(10000, 0.5f, fileRate);
    auto second = makeConstantTrack(10000, 0.5f, fileRate);
    Deck deck;
    deck.setTrack(first.get());

    press(deck, Button::Play);
    render(deck, 1000);
    press(deck, Button::Play);

    CHECK(deck.setTrack(second.get()) == first.get());
    CHECK(isSilent(render(deck, 100)));
}

TEST_CASE("Start, pause and CUE transitions have no click", "[engine][deck]")
{
    auto track = makeConstantTrack(100000, 0.5f, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    std::vector<float> samples;
    auto append = [&](int numSamples)
    {
        const auto out = render(deck, numSamples);

        for (int i = 0; i < numSamples; ++i)
            samples.push_back(out.getSample(0, i));
    };

    press(deck, Button::Play);
    append(1000);
    press(deck, Button::Play);
    append(1000);
    press(deck, Button::Play);
    append(1000);
    press(deck, Button::Cue);
    append(1000);
    release(deck, Button::Cue);
    append(1000);
    press(deck, Button::Cue);
    append(1000);
    release(deck, Button::Cue);
    append(1000);

    float maxJump = 0.0f;

    for (size_t i = 1; i < samples.size(); ++i)
        maxJump = juce::jmax(maxJump, std::abs(samples[i] - samples[i - 1]));

    CHECK(maxJump < 0.01f);
}

TEST_CASE("CUE while paused sets the cue and previews until release", "[engine][deck]")
{
    auto track = makeRampTrack(1000, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    render(deck, 50);
    press(deck, Button::Play);

    press(deck, Button::Cue);
    CHECK(deck.isPlaying());

    render(deck, 30);
    CHECK(positionSamples(deck) == Catch::Approx(80.0));

    release(deck, Button::Cue);
    CHECK_FALSE(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(50.0));
}

TEST_CASE("CUE while playing jumps to the cue and pauses", "[engine][deck]")
{
    auto track = makeRampTrack(1000, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    render(deck, 50);
    press(deck, Button::Play);
    press(deck, Button::Cue);
    release(deck, Button::Cue);

    press(deck, Button::Play);
    render(deck, 100);
    CHECK(positionSamples(deck) == Catch::Approx(150.0));

    press(deck, Button::Cue);
    CHECK_FALSE(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(50.0));

    release(deck, Button::Cue);
    CHECK_FALSE(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(50.0));
}

TEST_CASE("Play while holding CUE keeps playing after release", "[engine][deck]")
{
    auto track = makeRampTrack(1000, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Cue);
    render(deck, 10);

    press(deck, Button::Play);
    release(deck, Button::Cue);

    CHECK(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(10.0));

    render(deck, 10);
    CHECK(positionSamples(deck) == Catch::Approx(20.0));
}

TEST_CASE("Track end pauses at the end", "[engine][deck]")
{
    auto track = makeRampTrack(300, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    const auto out = render(deck, 400);

    CHECK_FALSE(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(300.0));
    CHECK(out.getSample(0, 299) == Catch::Approx(299.0f));
    CHECK(out.getSample(0, 300) == Catch::Approx(0.0f).margin(1.0e-9f));
}

TEST_CASE("Deck plays a 44.1 kHz track at 48 kHz at 44100/48000 per sample", "[engine][deck]")
{
    auto track = makeRampTrack(50000, 2, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    const auto out = render(deck, 480, 48000.0);

    CHECK(positionSamples(deck) == Catch::Approx(480.0 * fileRate / 48000.0));
    CHECK(out.getSample(0, 400) == Catch::Approx(400.0 * fileRate / 48000.0).margin(1.0e-3));
}

TEST_CASE("Mono track plays on both outputs", "[engine][deck]")
{
    auto track = makeRampTrack(100, 1, fileRate);
    Deck deck;
    deck.setTrack(track.get());

    press(deck, Button::Play);
    const auto out = render(deck, 10);

    for (int i = 0; i < 10; ++i)
        CHECK(out.getSample(0, i) == Catch::Approx(out.getSample(1, i)));
}

TEST_CASE("setTrack resets the position and returns the previous track", "[engine][deck]")
{
    auto first = makeRampTrack(1000, 2, fileRate);
    auto second = makeRampTrack(1000, 2, fileRate);
    Deck deck;

    CHECK(deck.setTrack(first.get()) == nullptr);
    press(deck, Button::Play);
    render(deck, 100);

    CHECK(deck.setTrack(second.get()) == first.get());
    CHECK_FALSE(deck.isPlaying());
    CHECK(positionSamples(deck) == Catch::Approx(0.0).margin(1.0e-9));
}
