#pragma once

#include <juce_core/juce_core.h>

#include <array>
#include <cstddef>

// Single producer, single consumer. Storage is fixed, so push and pop never
// allocate and are safe on the audio thread.
template <typename T, int Capacity>
class SpscQueue
{
public:
    bool push(const T& item)
    {
        const auto scope = fifo.write(1);

        if (scope.blockSize1 == 0)
            return false;

        storage[static_cast<std::size_t>(scope.startIndex1)] = item;
        return true;
    }

    bool pop(T& item)
    {
        const auto scope = fifo.read(1);

        if (scope.blockSize1 == 0)
            return false;

        item = storage[static_cast<std::size_t>(scope.startIndex1)];
        return true;
    }

private:
    // AbstractFifo keeps one slot free, so it needs one extra to hold Capacity.
    static constexpr int slots = Capacity + 1;

    juce::AbstractFifo fifo { slots };
    std::array<T, static_cast<std::size_t>(slots)> storage {};
};
