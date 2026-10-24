#pragma once

#include <atomic>

// Written by the audio thread, read by the UI. Seconds, not samples.
struct DeckSnapshot
{
    std::atomic<bool> loaded { false };
    std::atomic<bool> playing { false };
    std::atomic<double> positionSeconds { 0.0 };
    std::atomic<double> lengthSeconds { 0.0 };
};
