#include "ui/library/PanelResizer.h"

#include "ui/look/Tokens.h"

PanelResizer::PanelResizer(int min, int max, int defaultWidth, bool invert)
    : minWidth(min), maxWidth(max), inverted(invert), panelWidth(defaultWidth)
{
    setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    setWantsKeyboardFocus(false);
}

void PanelResizer::setPanelWidth(int width)
{
    width = juce::jlimit(minWidth, maxWidth, width);

    if (width == panelWidth)
        return;

    panelWidth = width;

    if (onResize)
        onResize();
}

void PanelResizer::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(dragging ? tokens::deckA : tokens::surfaceHover));
    g.fillRoundedRectangle(getLocalBounds().toFloat().withSizeKeepingCentre(
                               2.0f, static_cast<float>(getHeight())),
                           1.0f);
}

void PanelResizer::mouseDown(const juce::MouseEvent&)
{
    dragStartWidth = panelWidth;
    dragging = true;
    repaint();
}

void PanelResizer::mouseDrag(const juce::MouseEvent& event)
{
    const auto delta = event.getDistanceFromDragStartX();
    setPanelWidth(dragStartWidth + (inverted ? -delta : delta));
}

void PanelResizer::mouseUp(const juce::MouseEvent&)
{
    dragging = false;
    repaint();
}
