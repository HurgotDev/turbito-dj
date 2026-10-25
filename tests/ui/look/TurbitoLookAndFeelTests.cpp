#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    juce::Image paint(juce::Component& component, TurbitoLookAndFeel& lookAndFeel, int width,
                      int height)
    {
        component.setLookAndFeel(&lookAndFeel);
        component.setBounds(0, 0, width, height);

        juce::Image image(juce::Image::ARGB, width, height, true);
        juce::Graphics g(image);
        component.paintEntireComponent(g, true);
        component.setLookAndFeel(nullptr);
        return image;
    }
}

TEST_CASE("TurbitoLookAndFeel paints the stock widgets", "[ui][look]")
{
    const juce::ScopedJuceInitialiser_GUI juce;
    TurbitoLookAndFeel lookAndFeel;

    juce::TextButton button("Analizar");
    button.getProperties().set(TurbitoLookAndFeel::radiusProperty, tokens::radiusSmall);
    paint(button, lookAndFeel, 90, 36);

    button.setClickingTogglesState(true);
    button.setToggleState(true, juce::dontSendNotification);
    paint(button, lookAndFeel, 90, 40);

    juce::Slider knob(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    paint(knob, lookAndFeel, 26, 26);

    juce::Slider fader(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
    fader.setValue(3.0);
    paint(fader, lookAndFeel, 26, 126);

    juce::Slider crossfader(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    paint(crossfader, lookAndFeel, 200, 26);

    juce::ComboBox combo;
    combo.addItem("Echo", 1);
    combo.addItem("Reverb", 2);
    combo.setSelectedId(1, juce::dontSendNotification);
    paint(combo, lookAndFeel, 160, 26);
    paint(combo, lookAndFeel, 300, 36);

    juce::TableHeaderComponent header;
    header.addColumn("Artista", 1, 200);
    header.addColumn("BPM", 2, 60);
    const auto image = paint(header, lookAndFeel, 260, 30);

    CHECK(image.getPixelAt(250, 2).getARGB() == tokens::surface);
}

TEST_CASE("Knob marker points up at the middle value", "[ui][look]")
{
    const juce::ScopedJuceInitialiser_GUI juce;
    TurbitoLookAndFeel lookAndFeel;

    juce::Slider knob(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    knob.setRange(0.0, 1.0);
    knob.setValue(0.5);
    knob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                             juce::MathConstants<float>::pi * 2.75f, true);
    knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(tokens::deckA));

    const auto image = paint(knob, lookAndFeel, 26, 26);

    CHECK(image.getPixelAt(13, 5).getARGB() == tokens::deckA);
    CHECK(image.getPixelAt(13, 13).getARGB() == tokens::bg);
}
