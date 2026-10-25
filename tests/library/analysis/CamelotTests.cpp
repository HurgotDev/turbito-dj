#include <catch2/catch_test_macros.hpp>

#include "library/analysis/Camelot.h"

TEST_CASE("toCamelot maps all 24 qm-dsp keys to the Camelot wheel", "[library][analysis]")
{
    // qm-dsp GetKeyMode: 1-12 = C..B major, 13-24 = C..B minor.
    const char* const major[] = { "8B", "3B", "10B", "5B", "12B", "7B", "2B", "9B", "4B", "11B", "6B", "1B" };
    const char* const minor[] = { "5A", "12A", "7A", "2A", "9A", "4A", "11A", "6A", "1A", "8A", "3A", "10A" };

    for (int pitch = 0; pitch < 12; ++pitch)
    {
        CHECK(toCamelot(1 + pitch) == juce::String(major[pitch]));
        CHECK(toCamelot(13 + pitch) == juce::String(minor[pitch]));
    }
}

TEST_CASE("toCamelot returns an empty string when there is no key", "[library][analysis]")
{
    CHECK(toCamelot(0).isEmpty());
    CHECK(toCamelot(25).isEmpty());
    CHECK(toCamelot(-1).isEmpty());
}
