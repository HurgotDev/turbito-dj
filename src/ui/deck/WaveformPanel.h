#pragma once

#include <array>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/widgets/WaveformView.h"

// "Formas de onda": one lettered WaveformView per deck. A library row
// dropped on a deck's strip goes out through onFileDropped.
class WaveformPanel : public juce::Component, public juce::DragAndDropTarget
{
public:
    static constexpr int preferredHeight = 132;

    WaveformPanel();

    WaveformView& waveform(int deck);

    std::function<void(int deck, const juce::File&)> onFileDropped;

    bool isInterestedInDragSource(const SourceDetails&) override;
    void itemDropped(const SourceDetails&) override;

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    juce::Rectangle<int> tileBounds(int deck) const;

    std::array<WaveformView, 2> waveforms;
};
