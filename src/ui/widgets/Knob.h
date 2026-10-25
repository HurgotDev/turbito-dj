#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Angle of the marker for a 0–1 value: 270 degrees, centred on 12 o'clock.
float knobAngleDegrees(float value);

// Rotary control without a text box; double-click returns to the default.
class Knob : public juce::Slider
{
public:
    Knob(int diameter, double defaultValue);

    void setAccent(juce::Colour accent);
};
