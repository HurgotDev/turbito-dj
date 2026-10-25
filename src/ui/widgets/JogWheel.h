#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Degrees of the marker for a position: 33⅓ rpm, one turn per 1.8 s.
float jogAngleDegrees(double positionSeconds);

// Whole seconds as m:ss, e.g. "2:41".
juce::String formatElapsed(double seconds);

// Time left as -m:ss, e.g. "-3:31".
juce::String formatRemaining(double positionSeconds, double lengthSeconds);

// 148 px record with the elapsed and remaining time in the centre.
class JogWheel : public juce::Component
{
public:
    JogWheel();

    void setAccent(juce::Colour accent);
    void setPosition(double positionSeconds, double lengthSeconds);

    void paint(juce::Graphics&) override;

private:
    juce::Colour accent;
    double position = 0.0;
    double length = 0.0;
};
