#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

// Heights for `count` bars, one per 50 ms peak (0–255 → 0–48 px), centred
// on the playhead; bars before the start or past the end are 0.
std::vector<float> barsAround(const std::vector<std::uint8_t>& peaks, double positionSeconds,
                              int count = 160);

// 54 px waveform strip: bars left of the playhead in the played colour,
// the rest in the accent. Without bars it shows a flat placeholder.
class WaveformView : public juce::Component
{
public:
    WaveformView();

    void setColours(juce::Colour accent, juce::Colour played);

    // Bar heights in px; the strip is 54 px high. A 0 is no audio (before the
    // track starts): a flat bar in the empty colour.
    void setBars(std::vector<float> heights);

    // Loop region as fractions of the width.
    void setLoop(std::optional<juce::Range<float>> fractions);

    void paint(juce::Graphics&) override;

private:
    juce::Colour accent;
    juce::Colour played;
    std::vector<float> bars;
    std::optional<juce::Range<float>> loop;
};
