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
}
