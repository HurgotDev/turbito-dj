#include "ui/widgets/PadGrid.h"

#include "ui/Layout.h"
#include "ui/Strings.h"
#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    constexpr int tabsHeight = 28;
    constexpr int gap = 6;
    constexpr int columns = 4;
    constexpr int padRows = 2;
    constexpr int padCount = columns * padRows;
    using Index = std::size_t;
    constexpr float padRadius = static_cast<float>(tokens::radiusButton);

    PadGrid::Pad emptyPad()
    {
        return { juce::String::fromUTF8(strings::padEmpty), juce::Colour(tokens::empty),
                 juce::Colour(tokens::textMuted) };
    }
}

std::array<PadGrid::Pad, 8> PadGrid::defaultPads(Mode padMode, int selected)
{
    std::array<Pad, padCount> pads;

    for (Index i = 0; i < pads.size(); ++i)
    {
        const auto lit = static_cast<int>(i) == selected;

        switch (padMode)
        {
            case Mode::Loop:
                pads[i] = { strings::loopSizes[i], juce::Colour(lit ? tokens::loop : tokens::loopDim),
                            juce::Colour(lit ? tokens::bg : tokens::text) };
                break;
            case Mode::Fx:
                pads[i] = { strings::fxNames[i], juce::Colour(lit ? tokens::fx : tokens::fxDim),
                            juce::Colour(lit ? tokens::bg : tokens::text) };
                break;
            case Mode::HotCue:
            case Mode::Placas:
                pads[i] = emptyPad();
                break;
        }
    }

    return pads;
}

PadGrid::PadGrid()
    : accent(tokens::deckA)
{
    for (Index i = 0; i < tabs.size(); ++i)
    {
        auto& tab = tabs[i];
        tab.setButtonText(strings::padModes[i]);
        tab.setClickingTogglesState(true);
        tab.setRadioGroupId(1);
        tab.getProperties().set(TurbitoLookAndFeel::radiusProperty, tokens::radiusSmall);
        tab.onClick = [this, i]
        {
            setMode(static_cast<Mode>(i));

            if (onModeChanged)
                onModeChanged(mode);
        };
        addAndMakeVisible(tab);

        padsByMode[i] = defaultPads(static_cast<Mode>(i));
    }

    updateTabs();
}

void PadGrid::setAccent(juce::Colour newAccent)
{
    accent = newAccent;
    updateTabs();
}

void PadGrid::setMode(Mode newMode)
{
    mode = newMode;
    updateTabs();
}

void PadGrid::setPads(Mode padMode, std::array<Pad, 8> pads)
{
    padsByMode[static_cast<Index>(padMode)] = std::move(pads);
    repaint();
}

void PadGrid::updateTabs()
{
    for (Index i = 0; i < tabs.size(); ++i)
    {
        auto& tab = tabs[i];
        const auto selected = static_cast<int>(i) == static_cast<int>(mode);

        tab.setToggleState(selected, juce::dontSendNotification);
        tab.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        tab.setColour(juce::TextButton::buttonOnColourId, accent);
        tab.setColour(juce::TextButton::textColourOffId, juce::Colour(tokens::textSoft));
        tab.setColour(juce::TextButton::textColourOnId, juce::Colour(tokens::bg));
        tab.setColour(TurbitoLookAndFeel::buttonBorderColourId,
                      selected ? accent : juce::Colour(tokens::borderControl));
    }

    repaint();
}

juce::Rectangle<int> PadGrid::padBounds(int index) const
{
    const auto row = index / columns;
    const juce::Rectangle<int> rowBounds(0, tabsHeight + gap + row * (tokens::layoutPadHeight + gap),
                                         getWidth(), tokens::layoutPadHeight);

    return gridCell(rowBounds, columns, gap, index % columns);
}

int PadGrid::padAt(juce::Point<int> position) const
{
    for (int i = 0; i < padCount; ++i)
        if (padBounds(i).contains(position))
            return i;

    return -1;
}

void PadGrid::resized()
{
    for (Index i = 0; i < tabs.size(); ++i)
        tabs[i].setBounds(gridCell(getLocalBounds().withHeight(tabsHeight), columns, gap,
                                   static_cast<int>(i)));
}

void PadGrid::paint(juce::Graphics& g)
{
    const auto& pads = padsByMode[static_cast<Index>(mode)];

    for (int i = 0; i < padCount; ++i)
    {
        const auto& pad = pads[static_cast<Index>(i)];
        const auto bounds = padBounds(i).toFloat();

        g.setColour(pad.background);
        g.fillRoundedRectangle(bounds, padRadius);
        g.setColour(juce::Colour(tokens::borderControl));
        g.drawRoundedRectangle(bounds.reduced(0.5f), padRadius - 0.5f, 1.0f);

        const auto inner = bounds.reduced(9.0f, 6.0f);  // 8 × 5 padding inside the border
        g.setColour(pad.text);
        g.setFont(fonts::mono(static_cast<float>(tokens::fontSizeLabel), 700));
        // Glyphs centred in CSS line boxes: the number at line height 1, the label at 1.2.
        g.drawText(juce::String(i + 1), inner.withHeight(static_cast<float>(tokens::fontSizeLabel)),
                   juce::Justification::centredLeft, false);

        g.setFont(fonts::ui(static_cast<float>(tokens::fontSizeButton), 600));
        g.drawText(pad.label, inner.withTop(inner.getBottom() - 1.2f * tokens::fontSizeButton),
                   juce::Justification::centredLeft, true);
    }
}

void PadGrid::mouseDown(const juce::MouseEvent& event)
{
    pressedPad = padAt(event.getPosition());

    if (pressedPad >= 0 && onPad)
        onPad(pressedPad, true);
}

void PadGrid::mouseUp(const juce::MouseEvent&)
{
    if (pressedPad >= 0 && onPad)
        onPad(pressedPad, false);

    pressedPad = -1;
}
