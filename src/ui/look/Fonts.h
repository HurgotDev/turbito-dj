#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Embedded fonts. Sizes are CSS font-size (em, in component pixels); weights
// are CSS font-weight, taken from the variable fonts' wght axis. Message
// thread only.
namespace fonts
{
    juce::Typeface::Ptr uiTypeface(int weight = 400);
    juce::Typeface::Ptr monoTypeface(int weight = 500);

    juce::Font ui(float size, int weight = 400);
    juce::Font mono(float size, int weight = 500);
    juce::Font logo(float size);

    // CSS letter-spacing in em. JUCE's kerning factor is relative to the
    // font height (ascent + descent), not to the em size.
    juce::Font tracked(const juce::Font& font, float em);
}
