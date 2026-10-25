#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/TurbitoLookAndFeel.h"

// Paints a component at the given size with the app look and returns the pixels.
inline juce::Image paintWidget(juce::Component& component, int width, int height)
{
    TurbitoLookAndFeel lookAndFeel;
    component.setLookAndFeel(&lookAndFeel);
    component.setBounds(0, 0, width, height);

    juce::Image image(juce::Image::ARGB, width, height, true);
    {
        juce::Graphics g(image);
        component.paintEntireComponent(g, true);
    }

    component.setLookAndFeel(nullptr);
    return image;
}
