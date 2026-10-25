#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/deck/WaveformPanel.h"
#include "ui/look/Tokens.h"

TEST_CASE("Waveform panel stacks two 54 strips in 132", "[ui][deck]")
{
    const juce::ScopedJuceInitialiser_GUI juce;

    WaveformPanel panel;
    panel.setBounds(0, 0, 1420, WaveformPanel::preferredHeight);

    for (int deck = 0; deck < 2; ++deck)
    {
        const auto bounds = panel.waveform(deck).getBounds();
        CHECK(bounds.getHeight() == tokens::layoutWaveformHeight);
        CHECK(panel.getLocalBounds().reduced(9).contains(bounds));
    }
}
