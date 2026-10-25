#include "ui/mixer/MixerComponent.h"

#include "ui/Layout.h"
#include "ui/Strings.h"
#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    constexpr int inset = 1 + tokens::spacingPanelPadding;  // border + padding
    constexpr int sectionGap = 10;
    constexpr int channelGap = 10;
    constexpr int channelTitleHeight = 15;
    constexpr int channelTitleGap = 6;
    constexpr int knobSize = 26;
    constexpr int knobLabelHeight = 12;
    constexpr int knobStep = knobSize + 1 + knobLabelHeight + 3;
    constexpr int knobColumnHeight = 5 * knobStep - 3;
    constexpr int knobsToMeter = 10;
    constexpr int meterToFader = 6;
    constexpr int rangeMargin = 2;  // Chrome's margin around a range input
    constexpr int faderWidth = 26;
    constexpr int faderHeight = 126;
    constexpr int sectionPadding = 8;
    constexpr int fxRowHeight = 26;
    constexpr int fxRowGap = 6;
    constexpr int rowGap = 8;
    constexpr int dryWetSize = 24;
    constexpr int rangeRowHeight = 20;
    constexpr int placasValueWidth = 32;
    constexpr int selectedBeat = 1;  // "1/2", as in the mockup

    constexpr std::uint32_t accents[] = { tokens::deckA, tokens::deckB };
    constexpr Action knobActions[][5] = {
        { Action::ChannelA_Gain, Action::ChannelA_EqHigh, Action::ChannelA_EqMid,
          Action::ChannelA_EqLow, Action::ChannelA_Filter },
        { Action::ChannelB_Gain, Action::ChannelB_EqHigh, Action::ChannelB_EqMid,
          Action::ChannelB_EqLow, Action::ChannelB_Filter }
    };
    constexpr Action faderActions[] = { Action::ChannelA_Fader, Action::ChannelB_Fader };
    constexpr const char* knobNames[] = { strings::gain, strings::eqHigh, strings::eqMid,
                                          strings::eqLow, strings::filter };

    juce::Font knobLabelFont() { return fonts::ui(static_cast<float>(tokens::fontSizeLabel)); }
    juce::Font letterFont() { return fonts::ui(static_cast<float>(tokens::fontSizeButton), 700); }

    juce::Font labelFont()
    {
        return fonts::tracked(fonts::ui(static_cast<float>(tokens::fontSizeLabel)),
                              tokens::labelTracking);
    }

    int labelWidth(const char* text)
    {
        return textWidth(labelFont(), upperCase(juce::String::fromUTF8(text)));
    }
}

MixerComponent::MixerComponent()
{
    const auto connect = [this](juce::Slider& slider, Action action)
    {
        slider.onValueChange = [this, &slider, action]
        {
            if (onAction)
                onAction({ action, static_cast<float>(slider.getValue()) });
        };
    };

    for (std::size_t c = 0; c < channels.size(); ++c)
    {
        auto& channel = channels[c];
        const auto accent = juce::Colour(accents[c]);

        for (std::size_t k = 0; k < channel.knobs.size(); ++k)
        {
            channel.knobs[k].setAccent(accent);
            connect(channel.knobs[k], knobActions[c][k]);
            addAndMakeVisible(channel.knobs[k]);
        }

        addAndMakeVisible(channel.vu);

        channel.fader.setRange(0.0, 1.0);
        channel.fader.setValue(1.0, juce::dontSendNotification);
        channel.fader.setDoubleClickReturnValue(true, 1.0);
        channel.fader.setColour(juce::Slider::trackColourId, accent);
        connect(channel.fader, faderActions[c]);
        addAndMakeVisible(channel.fader);
    }

    // FX unit and Placas are drawn but send nothing until phases 8 and 9.
    for (int i = 0; i < 4; ++i)
        fxSelect.addItem(strings::fxNames[i], i + 1);
    fxSelect.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(fxSelect);

    dryWet.setAccent(juce::Colour(tokens::text));
    addAndMakeVisible(dryWet);

    for (std::size_t i = 0; i < fxBeats.size(); ++i)
    {
        auto& button = fxBeats[i];
        button.setButtonText(strings::fxBeats[i]);
        button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(tokens::text));
        button.setToggleState(static_cast<int>(i) == selectedBeat, juce::dontSendNotification);
        TurbitoLookAndFeel::setButtonFont(button, tokens::fontSizeLabel, 700, true);
    }

    for (std::size_t i = 0; i < fxTargets.size(); ++i)
    {
        fxTargets[i].setButtonText(juce::String::fromUTF8(strings::fxTargets[i]));
        TurbitoLookAndFeel::setButtonFont(fxTargets[i], tokens::fontSizeLabel, 700);
    }

    fxTargets[0].setColour(TurbitoLookAndFeel::buttonBorderColourId, juce::Colour(tokens::deckA));

    for (auto* button : { &fxBeats[0], &fxBeats[1], &fxBeats[2], &fxBeats[3], &fxTargets[0],
                          &fxTargets[1] })
    {
        button->getProperties().set(TurbitoLookAndFeel::radiusProperty, tokens::radiusSmall);
        addAndMakeVisible(button);
    }

    placas.setRange(0.0, 1.0);
    placas.setValue(1.0, juce::dontSendNotification);
    placas.setColour(juce::Slider::trackColourId, juce::Colour(tokens::placas[0]));
    placas.onValueChange = [this] { repaint(placasValue); };
    addAndMakeVisible(placas);

    crossfader.setRange(0.0, 1.0);
    crossfader.setValue(0.5, juce::dontSendNotification);
    crossfader.setDoubleClickReturnValue(true, 0.5);
    crossfader.setColour(juce::Slider::trackColourId, juce::Colour(tokens::text));
    connect(crossfader, Action::Mixer_Crossfader);
    addAndMakeVisible(crossfader);

    // Keys belong to the window's shortcuts, as in phase 3.
    for (auto* child : getChildren())
        child->setWantsKeyboardFocus(false);
}

void MixerComponent::setPeak(int deck, float peakLinear)
{
    channels[static_cast<std::size_t>(deck)].vu.setPeak(peakLinear);
}

juce::Slider* MixerComponent::sliderFor(Action action)
{
    for (std::size_t c = 0; c < channels.size(); ++c)
    {
        for (std::size_t k = 0; k < channels[c].knobs.size(); ++k)
            if (knobActions[c][k] == action)
                return &channels[c].knobs[k];

        if (faderActions[c] == action)
            return &channels[c].fader;
    }

    return action == Action::Mixer_Crossfader ? &crossfader : nullptr;
}

void MixerComponent::setControl(Action action, float value)
{
    if (auto* slider = sliderFor(action))
        slider->setValue(value, juce::dontSendNotification);
}

void MixerComponent::setPlacasVolume(float volume)
{
    placas.setValue(volume, juce::dontSendNotification);
    repaint(placasValue);
}

void MixerComponent::resized()
{
    auto area = getLocalBounds().reduced(inset);

    // Channels: title, then knobs | VU | fader, centred and bottom-aligned.
    const auto channelRow = area.removeFromTop(channelTitleHeight + channelTitleGap
                                               + knobColumnHeight);
    auto knobColumnWidth = knobSize;

    for (const auto* name : knobNames)
        knobColumnWidth = juce::jmax(knobColumnWidth, textWidth(knobLabelFont(), name));

    const auto groupWidth = knobColumnWidth + knobsToMeter + channels[0].vu.getWidth()
                            + meterToFader + faderWidth + 2 * rangeMargin;

    for (std::size_t c = 0; c < channels.size(); ++c)
    {
        auto& channel = channels[c];
        auto column = gridCell(channelRow, 2, channelGap, static_cast<int>(c));
        channel.title = column.removeFromTop(channelTitleHeight);
        column.removeFromTop(channelTitleGap);

        auto group = column.withSizeKeepingCentre(groupWidth, column.getHeight());
        auto knobs = group.removeFromLeft(knobColumnWidth);

        for (std::size_t k = 0; k < channel.knobs.size(); ++k)
        {
            auto cell = knobs.removeFromTop(knobStep);
            channel.knobs[k].setBounds(cell.removeFromTop(knobSize).withSizeKeepingCentre(knobSize,
                                                                                         knobSize));
            cell.removeFromTop(1);
            channel.knobLabels[k] = cell.removeFromTop(knobLabelHeight);
        }

        group.removeFromLeft(knobsToMeter);
        const auto meterHeight = channel.vu.getHeight();
        channel.vu.setBounds(group.removeFromLeft(channel.vu.getWidth())
                                 .removeFromBottom(meterHeight));
        group.removeFromLeft(meterToFader);
        channel.fader.setBounds(group.removeFromBottom(faderHeight + 2 * rangeMargin)
                                    .reduced(rangeMargin));
    }

    // FX: select and Dry/Wet, then beats and targets.
    area.removeFromTop(sectionGap);
    fxSeparator = area.removeFromTop(1);
    area.removeFromTop(sectionPadding);

    auto fxRow = area.removeFromTop(fxRowHeight);
    fxLabel = fxRow.removeFromLeft(labelWidth(strings::fx));
    fxRow.removeFromLeft(rowGap);
    dryWet.setBounds(fxRow.removeFromRight(dryWetSize).withSizeKeepingCentre(dryWetSize, dryWetSize));
    fxRow.removeFromRight(rowGap);
    fxSelect.setBounds(fxRow);
    area.removeFromTop(fxRowGap);

    const auto beatsRow = area.removeFromTop(fxRowHeight);
    for (std::size_t i = 0; i < fxBeats.size() + fxTargets.size(); ++i)
    {
        auto& button = i < fxBeats.size() ? fxBeats[i] : fxTargets[i - fxBeats.size()];
        button.setBounds(gridCell(beatsRow, 6, 4, static_cast<int>(i)));
    }

    // Placas volume.
    area.removeFromTop(sectionGap);
    placasSeparator = area.removeFromTop(1);
    area.removeFromTop(sectionPadding);

    auto placasRow = area.removeFromTop(rangeRowHeight);
    placasLabel = placasRow.removeFromLeft(labelWidth(strings::placas));
    placasRow.removeFromLeft(rowGap);
    placasValue = placasRow.removeFromRight(placasValueWidth);
    placasRow.removeFromRight(rowGap);
    placas.setBounds(placasRow.reduced(rangeMargin));

    // Crossfader.
    area.removeFromTop(sectionGap);
    auto crossfaderRow = area.removeFromTop(rangeRowHeight);
    letterA = crossfaderRow.removeFromLeft(textWidth(letterFont(), strings::deckLetters[0]));
    crossfaderRow.removeFromLeft(rowGap);
    letterB = crossfaderRow.removeFromRight(textWidth(letterFont(), strings::deckLetters[1]));
    crossfaderRow.removeFromRight(rowGap);
    crossfader.setBounds(crossfaderRow.reduced(rangeMargin));
}

void MixerComponent::paint(juce::Graphics& g)
{
    TurbitoLookAndFeel::drawPanel(g, getLocalBounds().toFloat());

    for (std::size_t c = 0; c < channels.size(); ++c)
    {
        const auto& channel = channels[c];

        g.setColour(juce::Colour(accents[c]));
        g.setFont(letterFont());
        g.drawText(strings::channelNames[c], channel.title, juce::Justification::centred, false);

        g.setColour(juce::Colour(tokens::textMuted));
        g.setFont(knobLabelFont());

        for (std::size_t k = 0; k < channel.knobLabels.size(); ++k)
            g.drawText(knobNames[k], channel.knobLabels[k], juce::Justification::centred, false);
    }

    g.setColour(juce::Colour(tokens::border));
    g.fillRect(fxSeparator);
    g.fillRect(placasSeparator);

    TurbitoLookAndFeel::drawLabelText(g, strings::fx, fxLabel.toFloat(),
                                      juce::Justification::centredLeft);
    TurbitoLookAndFeel::drawLabelText(g, strings::placas, placasLabel.toFloat(),
                                      juce::Justification::centredLeft);

    g.setColour(juce::Colour(tokens::text));
    g.setFont(fonts::mono(static_cast<float>(tokens::fontSizeLabel)));
    g.drawText(juce::String(juce::roundToInt(placas.getValue() * 100.0)) + "%", placasValue,
               juce::Justification::centredRight, false);

    g.setFont(letterFont());
    g.setColour(juce::Colour(tokens::deckA));
    g.drawText(strings::deckLetters[0], letterA, juce::Justification::centredLeft, false);
    g.setColour(juce::Colour(tokens::deckB));
    g.drawText(strings::deckLetters[1], letterB, juce::Justification::centredRight, false);
}
