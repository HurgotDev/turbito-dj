#include <catch2/catch_test_macros.hpp>

#include "core/SpscQueue.h"

TEST_CASE("SpscQueue pops in push order", "[core][spsc]")
{
    SpscQueue<int, 4> queue;

    REQUIRE(queue.push(1));
    REQUIRE(queue.push(2));
    REQUIRE(queue.push(3));

    int value = 0;
    REQUIRE(queue.pop(value));
    CHECK(value == 1);
    REQUIRE(queue.pop(value));
    CHECK(value == 2);
    REQUIRE(queue.pop(value));
    CHECK(value == 3);
}

TEST_CASE("SpscQueue rejects push when full", "[core][spsc]")
{
    SpscQueue<int, 2> queue;

    REQUIRE(queue.push(10));
    REQUIRE(queue.push(20));
    CHECK_FALSE(queue.push(30));

    int value = 0;
    REQUIRE(queue.pop(value));
    CHECK(value == 10);
    CHECK(queue.push(30));
}

TEST_CASE("SpscQueue rejects pop when empty", "[core][spsc]")
{
    SpscQueue<int, 2> queue;

    int value = 42;
    CHECK_FALSE(queue.pop(value));
    CHECK(value == 42);
}

TEST_CASE("SpscQueue keeps order across index wrap-around", "[core][spsc]")
{
    SpscQueue<int, 3> queue;
    int next = 0;
    int expected = 0;

    for (int round = 0; round < 10; ++round)
    {
        for (int i = 0; i < 3; ++i)
            REQUIRE(queue.push(next++));

        for (int i = 0; i < 3; ++i)
        {
            int value = -1;
            REQUIRE(queue.pop(value));
            CHECK(value == expected++);
        }
    }

    CHECK(expected == 30);
}
