#pragma once

#include <cmath>

#include <juce_gui_basics/juce_gui_basics.h>

// Cell `index` of a row split into `count` equal columns with `gap` between
// them, like a CSS grid of 1fr columns.
inline juce::Rectangle<int> gridCell(juce::Rectangle<int> row, int count, int gap, int index)
{
    const auto width = static_cast<float>(row.getWidth() - gap * (count - 1))
                       / static_cast<float>(count);
    const auto start = static_cast<float>(index) * (width + static_cast<float>(gap));
    const auto left = juce::roundToInt(start);
    const auto right = juce::roundToInt(start + width);
    return row.withX(row.getX() + left).withWidth(right - left);
}

// Width of a single line of text, rounded up.
inline int textWidth(const juce::Font& font, const juce::String& text)
{
    return static_cast<int>(std::ceil(juce::GlyphArrangement::getStringWidth(font, text)));
}

// Uppercase that also covers á, é, í, ó, ú, ñ, ü: juce::String::toUpperCase
// goes through towupper, which leaves non-ASCII letters alone in the C locale.
inline juce::String upperCase(const juce::String& text)
{
    juce::String result;
    result.preallocateBytes(text.getNumBytesAsUTF8());

    for (auto p = text.getCharPointer(); ! p.isEmpty();)
    {
        auto c = juce::CharacterFunctions::toUpperCase(p.getAndAdvance());

        if (c >= 0xe0 && c <= 0xfe && c != 0xf7)  // Latin-1 lowercase → uppercase
            c -= 0x20;

        result += juce::String::charToString(c);
    }

    return result;
}
