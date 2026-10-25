#pragma once

#include <array>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/widgets/WaveformView.h"

// "Formas de onda": one lettered WaveformView per deck.
class WaveformPanel : public juce::Component
{
public:
    static constexpr int preferredHeight = 132;

    WaveformPanel();

    WaveformView& waveform(int deck);

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    juce::Rectangle<int> tileBounds(int deck) const;

    std::array<WaveformView, 2> waveforms;
};
