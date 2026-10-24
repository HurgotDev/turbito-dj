#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <functional>
#include <vector>

#include "engine/mixer/Channel.h"

namespace
{
    constexpr double sampleRate = 44100.0;
    constexpr float amplitude = 0.5f;

    // Gain in dB of a sine through the channel: RMS over 0.2 s after 0.2 s of
    // settling, relative to the input RMS.
    double gainDb(double frequency, const std::function<void(Channel&)>& setup = {})
    {
        Channel channel;
        channel.prepare(sampleRate);

        if (setup)
            setup(channel);

        const int settle = static_cast<int>(sampleRate * 0.2);
        const int total = settle * 2;
        std::vector<float> left(static_cast<size_t>(total)), right(static_cast<size_t>(total));

        for (int i = 0; i < total; ++i)
            left[static_cast<size_t>(i)] = right[static_cast<size_t>(i)] = amplitude
                * static_cast<float>(std::sin(juce::MathConstants<double>::twoPi * frequency * i / sampleRate));

        for (int offset = 0; offset < total; offset += 512)
            channel.process(left.data() + offset, right.data() + offset, juce::jmin(512, total - offset));

        double sum = 0.0;

        for (int i = settle; i < total; ++i)
            sum += static_cast<double>(left[static_cast<size_t>(i)]) * left[static_cast<size_t>(i)];

        const double rms = std::sqrt(sum / (total - settle));
        return juce::Decibels::gainToDecibels(rms / (amplitude / std::sqrt(2.0)), -200.0);
    }
}

TEST_CASE("Channel at defaults passes every band flat", "[engine][mixer]")
{
    for (double frequency : { 50.0, 1000.0, 10000.0 })
        CHECK(gainDb(frequency) == Catch::Approx(0.0).margin(0.1));
}

TEST_CASE("Channel EQ kills its band and leaves the others", "[engine][mixer]")
{
    const auto killLow = [](Channel& c) { c.setEqLow(0.0f); };
    CHECK(gainDb(50.0, killLow) < -40.0);
    CHECK(gainDb(1000.0, killLow) == Catch::Approx(0.0).margin(1.0));
    CHECK(gainDb(10000.0, killLow) == Catch::Approx(0.0).margin(1.0));

    const auto killHigh = [](Channel& c) { c.setEqHigh(0.0f); };
    CHECK(gainDb(10000.0, killHigh) < -40.0);
    CHECK(gainDb(50.0, killHigh) == Catch::Approx(0.0).margin(1.0));
    CHECK(gainDb(1000.0, killHigh) == Catch::Approx(0.0).margin(1.0));

    const auto killMid = [](Channel& c) { c.setEqMid(0.0f); };
    CHECK(gainDb(1000.0, killMid) < -20.0);
    CHECK(gainDb(50.0, killMid) == Catch::Approx(0.0).margin(1.0));
    CHECK(gainDb(10000.0, killMid) == Catch::Approx(0.0).margin(1.0));
}

TEST_CASE("Channel EQ at maximum boosts its band by 6 dB", "[engine][mixer]")
{
    CHECK(gainDb(50.0, [](Channel& c) { c.setEqLow(1.0f); }) == Catch::Approx(6.0).margin(0.5));
    CHECK(gainDb(1000.0, [](Channel& c) { c.setEqMid(1.0f); }) == Catch::Approx(6.0).margin(0.5));
    CHECK(gainDb(10000.0, [](Channel& c) { c.setEqHigh(1.0f); }) == Catch::Approx(6.0).margin(0.5));
}

TEST_CASE("Channel gain spans -12 to +12 dB and fader 0 is silence", "[engine][mixer]")
{
    CHECK(gainDb(1000.0, [](Channel& c) { c.setGain(1.0f); }) == Catch::Approx(12.0).margin(0.1));
    CHECK(gainDb(1000.0, [](Channel& c) { c.setGain(0.0f); }) == Catch::Approx(-12.0).margin(0.1));
    CHECK(gainDb(1000.0, [](Channel& c) { c.setFader(0.0f); }) < -120.0);
}

TEST_CASE("Channel filter knob sweeps low-pass and high-pass", "[engine][mixer]")
{
    const auto lowPass = [](Channel& c) { c.setFilter(0.1f); };
    CHECK(gainDb(8000.0, lowPass) < -20.0);
    CHECK(gainDb(100.0, lowPass) == Catch::Approx(0.0).margin(1.0));

    const auto highPass = [](Channel& c) { c.setFilter(0.9f); };
    CHECK(gainDb(100.0, highPass) < -20.0);
    CHECK(gainDb(8000.0, highPass) == Catch::Approx(0.0).margin(1.0));

    const auto centre = [](Channel& c) { c.setFilter(0.5f); };
    CHECK(gainDb(100.0, centre) == Catch::Approx(0.0).margin(0.1));
    CHECK(gainDb(8000.0, centre) == Catch::Approx(0.0).margin(0.1));
}

TEST_CASE("Channel peak follows the fader", "[engine][mixer]")
{
    Channel channel;
    channel.prepare(sampleRate);

    const auto peakAfter = [&](float fader)
    {
        channel.setFader(fader);
        std::vector<float> left(4410, 0.0f), right(4410, 0.0f);

        // A long constant signal: the EQ passes DC at unity.
        for (int block = 0; block < 10; ++block)
        {
            std::fill(left.begin(), left.end(), amplitude);
            std::fill(right.begin(), right.end(), amplitude);
            channel.process(left.data(), right.data(), static_cast<int>(left.size()));
        }

        return channel.peak();
    };

    CHECK(peakAfter(1.0f) == Catch::Approx(amplitude).margin(1.0e-3));
    CHECK(peakAfter(0.5f) == Catch::Approx(amplitude * 0.25f).margin(1.0e-3));
    CHECK(peakAfter(0.0f) == Catch::Approx(0.0f).margin(1.0e-6));
}
