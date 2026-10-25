#pragma once

#include <juce_core/juce_core.h>

// keyIndex as returned by qm-dsp GetKeyMode: 1-12 = C..B major, 13-24 = C..B minor.
// Returns "" for 0 (no key) or any other value.
juce::String toCamelot(int keyIndex);
