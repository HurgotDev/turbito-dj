#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

// The mockup's 12-wide panel resizer: a 2px line that turns deckA while
// dragging. Holds the width of the panel it resizes, clamped to its range;
// `inverted` is for a panel on the right, which widens when dragged left.
class PanelResizer : public juce::Component
{
public:
    static constexpr int preferredWidth = 12;

    PanelResizer(int minWidth, int maxWidth, int defaultWidth, bool inverted = false);

    int getPanelWidth() const noexcept { return panelWidth; }
    void setPanelWidth(int width);

    std::function<void()> onResize;

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    const int minWidth, maxWidth;
    const bool inverted;
    int panelWidth;
    int dragStartWidth = 0;
    bool dragging = false;
};
