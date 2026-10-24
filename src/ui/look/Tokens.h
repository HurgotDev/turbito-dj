#pragma once

#include <cstdint>

// shortcut: hand-copied from design/tokens.json, generate from JSON in phase 4
namespace tokens
{
    struct Size
    {
        int width;
        int height;
    };

    inline constexpr Size windowReference { 1440, 900 };
    inline constexpr Size windowMinimum { 1280, 720 };
    inline constexpr std::uint32_t bg = 0xff0A0A12;
    inline constexpr std::uint32_t surface = 0xff1D1A2B;
    inline constexpr std::uint32_t vuLow = 0xff39FF6A;
    inline constexpr std::uint32_t vuMid = 0xffFFE500;
    inline constexpr std::uint32_t vuHigh = 0xffFF2D3F;
}
