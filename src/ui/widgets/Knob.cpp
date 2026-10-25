#include "ui/widgets/Knob.h"

namespace
{
    constexpr double sweep = juce::MathConstants<double>::pi * 0.75;  // ±135°
}

float knobAngleDegrees(float value)
{
    return (value - 0.5f) * 270.0f;
}

Knob::Knob(int diameter, double defaultValue)
    : juce::Slider(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox)
{
    setRange(0.0, 1.0, 0.0);
    // JUCE wants non-negative angles; a full turn ahead draws the same.
    constexpr auto turn = juce::MathConstants<double>::twoPi;
    setRotaryParameters(static_cast<float>(turn - sweep), static_cast<float>(turn + sweep), true);
    setDoubleClickReturnValue(true, defaultValue);
    setValue(defaultValue, juce::dontSendNotification);
    setSize(diameter, diameter);
}

void Knob::setAccent(juce::Colour accent)
{
    setColour(juce::Slider::rotarySliderFillColourId, accent);
}
