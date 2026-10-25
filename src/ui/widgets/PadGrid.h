#pragma once

#include <array>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

// Mode tabs over a 4 × 2 pad grid. Each mode keeps its own pad content;
// the tab only chooses which one is shown.
class PadGrid : public juce::Component
{
public:
    enum class Mode { HotCue, Loop, Fx, Placas };

    struct Pad
    {
        juce::String label;
        juce::Colour background;
        juce::Colour text;
    };

    static constexpr int totalHeight = 128;

    // Mockup content of a mode; selected is the lit pad of Loop and FX.
    static std::array<Pad, 8> defaultPads(Mode mode, int selected = 0);

    PadGrid();

    void setAccent(juce::Colour accent);
    void setMode(Mode mode);
    void setPads(Mode mode, std::array<Pad, 8> pads);

    std::function<void(Mode)> onModeChanged;
    std::function<void(int index, bool down)> onPad;

    void resized() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    juce::Rectangle<int> padBounds(int index) const;
    int padAt(juce::Point<int> position) const;
    void updateTabs();

    std::array<juce::TextButton, 4> tabs;
    std::array<std::array<Pad, 8>, 4> padsByMode;
    juce::Colour accent;
    Mode mode = Mode::HotCue;
    int pressedPad = -1;
};
