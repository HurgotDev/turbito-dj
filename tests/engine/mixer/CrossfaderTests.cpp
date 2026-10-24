#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "engine/mixer/Crossfader.h"

TEST_CASE("Crossfader ends are full A and full B", "[engine][mixer]")
{
    for (float curve : { 0.0f, 0.5f, 1.0f })
    {
        const auto left = crossfaderGains(0.0f, curve);
        const auto right = crossfaderGains(1.0f, curve);

        CHECK(left.a == Catch::Approx(1.0f));
        CHECK(left.b == Catch::Approx(0.0f).margin(1.0e-6f));
        CHECK(right.a == Catch::Approx(0.0f).margin(1.0e-6f));
        CHECK(right.b == Catch::Approx(1.0f));
    }
}

TEST_CASE("Crossfader curve 0 keeps constant power", "[engine][mixer]")
{
    for (int i = 0; i <= 100; ++i)
    {
        const auto gains = crossfaderGains(static_cast<float>(i) / 100.0f, 0.0f);
        CHECK(gains.a * gains.a + gains.b * gains.b == Catch::Approx(1.0f).margin(1.0e-5f));
    }
}

TEST_CASE("Crossfader curve 1 has both sides full at the centre", "[engine][mixer]")
{
    const auto gains = crossfaderGains(0.5f, 1.0f);

    CHECK(gains.a == Catch::Approx(1.0f));
    CHECK(gains.b == Catch::Approx(1.0f));
}

TEST_CASE("Crossfader gains are monotonic", "[engine][mixer]")
{
    for (float curve : { 0.0f, 0.3f, 1.0f })
    {
        auto previous = crossfaderGains(0.0f, curve);

        for (int i = 1; i <= 100; ++i)
        {
            const auto gains = crossfaderGains(static_cast<float>(i) / 100.0f, curve);

            CHECK(gains.a <= previous.a);
            CHECK(gains.b >= previous.b);
            previous = gains;
        }
    }
}
