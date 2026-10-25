#include "library/analysis/Camelot.h"

juce::String toCamelot(int keyIndex)
{
    if (keyIndex < 1 || keyIndex > 24)
        return {};

    const bool minor = keyIndex > 12;
    const int pitchClass = (keyIndex - 1) % 12;

    // Each fifth up is one step on the wheel; C major = 8B, A minor (relative of C) = 8A.
    const int number = ((minor ? 5 : 8) + 7 * pitchClass) % 12;
    return juce::String(number == 0 ? 12 : number) + (minor ? "A" : "B");
}
