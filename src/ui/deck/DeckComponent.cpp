#include "ui/deck/DeckComponent.h"

#include <cmath>

#include "ui/Layout.h"
#include "ui/Strings.h"
#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    constexpr int inset = 1 + tokens::spacingPanelPadding;  // border + padding
    constexpr int columnGap = 10;
    constexpr int headerHeight = 38;
    constexpr int headerGap = 12;
    constexpr int tileSize = 36;
    constexpr int progressHeight = 4;
    constexpr int jogRowHeight = 152;
    constexpr int jogRowGap = 14;
    constexpr int transportHeight = 40;
    constexpr int syncHeight = 30;
    constexpr int loopHeight = 28;
    constexpr int middleGap = 6;
    constexpr int loopGap = 4;
    constexpr int pitchTextHeight = 14;
    constexpr int pitchMargin = 2;  // Chrome's margin around a range input
    constexpr int pitchWidth = 26;
    constexpr int pitchHeight = 112;
    constexpr int selectedLoop = 3;  // "4", as in the mockup

    constexpr std::uint32_t accents[] = { tokens::deckA, tokens::deckB };
    constexpr Action playActions[] = { Action::DeckA_Play, Action::DeckB_Play };
    constexpr Action cueActions[] = { Action::DeckA_Cue, Action::DeckB_Cue };

    juce::Font pitchLabelFont()
    {
        return fonts::tracked(fonts::ui(static_cast<float>(tokens::fontSizeLabel)),
                              tokens::labelTracking);
    }
}

juce::String formatPitch(float percent)
{
    // Round first so slider noise (1e-15) and -0.0 both read "0.0%".
    const auto rounded = std::round(percent * 10.0f) / 10.0f + 0.0f;
    return (rounded > 0.0f ? "+" : "") + juce::String(rounded, 1) + "%";
}

DeckComponent::DeckComponent(int deckIndex)
    : deck(deckIndex),
      accent(accents[deckIndex]),
      title(strings::noTrack),
      bpm(juce::String::fromUTF8(strings::noValue)),
      key(juce::String::fromUTF8(strings::noValue))
{
    jog.setAccent(accent);
    padGrid.setAccent(accent);

    cue.setButtonText(strings::cue);
    sync.setButtonText(strings::sync);
    keylock.setButtonText(strings::keylock);
    play.setColour(TurbitoLookAndFeel::buttonBorderColourId, accent);
    sync.setColour(juce::TextButton::buttonOnColourId, accent);
    setPlaying(false);

    // Same semantics as phase 3: play toggles on each click, cue is held.
    play.onClick = [this] { emit(playActions[deck], 1.0f); };
    cue.onStateChange = [this]
    {
        // Repaints call this again without a transition.
        if (cue.isDown() == cueDown)
            return;

        cueDown = cue.isDown();
        emit(cueActions[deck], cueDown ? 1.0f : 0.0f);
    };

    for (std::size_t i = 0; i < loopSizes.size(); ++i)
    {
        auto& button = loopSizes[i];
        button.setButtonText(strings::loopButtons[i]);
        button.setColour(juce::TextButton::buttonOnColourId, accent);
        button.setToggleState(static_cast<int>(i) == selectedLoop, juce::dontSendNotification);
        TurbitoLookAndFeel::setButtonFont(button, tokens::fontSizeLabel, 700, true);
    }

    const char* controlNames[] = { strings::loopIn, strings::loopOut, strings::reloop };

    for (std::size_t i = 0; i < loopControls.size(); ++i)
    {
        loopControls[i].setButtonText(controlNames[i]);
        TurbitoLookAndFeel::setButtonFont(loopControls[i], tokens::fontSizeLabel, 600);
    }

    for (auto& button : loopSizes)
        button.getProperties().set(TurbitoLookAndFeel::radiusProperty, tokens::radiusSmall);

    for (auto& button : loopControls)
        button.getProperties().set(TurbitoLookAndFeel::radiusProperty, tokens::radiusSmall);

    // Phase 4 draws pitch without sending it (phase 6).
    pitch.setSliderStyle(juce::Slider::LinearVertical);
    pitch.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pitch.setRange(-8.0, 8.0, 0.1);
    pitch.setValue(0.0, juce::dontSendNotification);
    pitch.setColour(juce::Slider::trackColourId, accent);
    pitch.onValueChange = [this] { repaint(pitchValue); };

    for (auto* child : std::initializer_list<juce::Component*> { &jog, &cue, &play, &sync,
                                                                 &keylock, &pitch, &padGrid })
        addAndMakeVisible(child);

    for (auto& button : loopSizes)
        addAndMakeVisible(button);

    for (auto& button : loopControls)
        addAndMakeVisible(button);

    // Keys belong to the window's shortcuts, as in phase 3.
    for (auto* child : getChildren())
        child->setWantsKeyboardFocus(false);
}

void DeckComponent::setTrack(const juce::String& newTitle, const juce::String& newArtist)
{
    title = newTitle;
    artist = newArtist;
    repaint(header);
}

void DeckComponent::setBpm(const juce::String& newBpm)
{
    bpm = newBpm;
    repaint(header);
}

void DeckComponent::setKey(const juce::String& newKey)
{
    key = newKey;
    repaint(header);
}

void DeckComponent::setPlaying(bool playing)
{
    play.setToggleState(playing, juce::dontSendNotification);
    play.setButtonText(playing ? strings::pause : strings::play);
}

void DeckComponent::setPosition(double positionSeconds, double lengthSeconds)
{
    position = positionSeconds;
    length = lengthSeconds;
    jog.setPosition(positionSeconds, lengthSeconds);
    repaint(progress);
}

void DeckComponent::setPitch(float percent)
{
    pitch.setValue(percent, juce::dontSendNotification);
    repaint(pitchValue);
}

void DeckComponent::setSync(bool on)
{
    sync.setToggleState(on, juce::dontSendNotification);
}

PadGrid& DeckComponent::pads()
{
    return padGrid;
}

void DeckComponent::emit(Action action, float value)
{
    if (onAction)
        onAction({ action, value });
}

void DeckComponent::resized()
{
    auto area = getLocalBounds().reduced(inset);

    header = area.removeFromTop(headerHeight);
    area.removeFromTop(columnGap);
    progress = area.removeFromTop(progressHeight);
    area.removeFromTop(columnGap);
    auto row = area.removeFromTop(jogRowHeight);
    area.removeFromTop(columnGap);
    padGrid.setBounds(area.removeFromTop(PadGrid::totalHeight));

    jog.setBounds(row.removeFromLeft(tokens::layoutJogSize)
                      .withSizeKeepingCentre(tokens::layoutJogSize, tokens::layoutJogSize));
    row.removeFromLeft(jogRowGap);

    const auto pitchColumnWidth = juce::jmax(pitchWidth + 2 * pitchMargin,
                                             textWidth(pitchLabelFont(),
                                                       juce::String(strings::pitch).toUpperCase()));
    auto pitchColumn = row.removeFromRight(pitchColumnWidth);
    row.removeFromRight(jogRowGap);

    pitchLabel = pitchColumn.removeFromTop(pitchTextHeight);
    pitchValue = pitchColumn.removeFromBottom(pitchTextHeight);
    pitch.setBounds(pitchColumn.withSizeKeepingCentre(pitchWidth, pitchHeight));

    constexpr int middleHeight = transportHeight + syncHeight + 2 * loopHeight + 3 * middleGap;
    auto middle = row.withSizeKeepingCentre(row.getWidth(), middleHeight);

    const auto transport = middle.removeFromTop(transportHeight);
    cue.setBounds(gridCell(transport, 2, middleGap, 0));
    play.setBounds(gridCell(transport, 2, middleGap, 1));
    middle.removeFromTop(middleGap);

    const auto syncRow = middle.removeFromTop(syncHeight);
    sync.setBounds(gridCell(syncRow, 2, middleGap, 0));
    keylock.setBounds(gridCell(syncRow, 2, middleGap, 1));
    middle.removeFromTop(middleGap);

    const auto sizesRow = middle.removeFromTop(loopHeight);
    for (std::size_t i = 0; i < loopSizes.size(); ++i)
        loopSizes[i].setBounds(gridCell(sizesRow, 6, loopGap, static_cast<int>(i)));
    middle.removeFromTop(middleGap);

    for (std::size_t i = 0; i < loopControls.size(); ++i)
        loopControls[i].setBounds(gridCell(middle, 3, loopGap, static_cast<int>(i)));
}

void DeckComponent::paint(juce::Graphics& g)
{
    TurbitoLookAndFeel::drawPanel(g, getLocalBounds().toFloat());

    // Header: letter tile, title and artist, BPM and key.
    auto row = header;
    const auto tile = row.removeFromLeft(tileSize).withSizeKeepingCentre(tileSize, tileSize).toFloat();
    g.setColour(accent);
    g.fillRoundedRectangle(tile, static_cast<float>(tokens::radiusButton));
    g.setColour(juce::Colour(tokens::bg));
    g.setFont(fonts::ui(16.0f, 700));
    g.drawText(strings::deckLetters[deck], tile, juce::Justification::centred, false);
    row.removeFromLeft(headerGap);

    const auto bpmFont = fonts::mono(static_cast<float>(tokens::fontSizeBpm), 700);
    const auto keyFont = fonts::ui(static_cast<float>(tokens::fontSizeLabel));
    const auto keyText = juce::String::fromUTF8(strings::bpmAndKey) + key;
    auto right = row.removeFromRight(juce::jmax(textWidth(bpmFont, bpm), textWidth(keyFont, keyText)));
    row.removeFromRight(headerGap);

    g.setColour(juce::Colour(tokens::text));
    g.setFont(bpmFont);
    g.drawText(bpm, right.removeFromTop(tokens::fontSizeBpm), juce::Justification::centredRight, false);
    right.removeFromTop(3);
    g.setColour(juce::Colour(tokens::textMuted));
    g.setFont(keyFont);
    g.drawText(keyText, right, juce::Justification::centredRight, false);

    // Title 15 and artist 12 at CSS line height (≈ 1.2), centred in the row.
    auto text = row.withSizeKeepingCentre(row.getWidth(), 33);
    g.setColour(juce::Colour(tokens::text));
    g.setFont(fonts::ui(static_cast<float>(tokens::fontSizeTrackTitle), 600));
    g.drawText(title, text.removeFromTop(18), juce::Justification::centredLeft, true);
    g.setColour(juce::Colour(tokens::textMuted));
    g.setFont(fonts::ui(static_cast<float>(tokens::fontSizeButton)));
    g.drawText(artist, text, juce::Justification::centredLeft, true);

    // Progress: a square-ended fill clipped by the rounded track.
    {
        const auto bar = progress.toFloat();
        const auto fraction = length > 0.0 ? juce::jlimit(0.0, 1.0, position / length) : 0.0;
        juce::Path track;
        track.addRoundedRectangle(bar, bar.getHeight() / 2);

        const juce::Graphics::ScopedSaveState state(g);
        g.reduceClipRegion(track);
        g.setColour(juce::Colour(tokens::border));
        g.fillRect(bar);
        g.setColour(accent);
        g.fillRect(bar.withWidth(bar.getWidth() * static_cast<float>(fraction)));
    }

    // Pitch label and value.
    TurbitoLookAndFeel::drawLabelText(g, strings::pitch, pitchLabel.toFloat(),
                                      juce::Justification::centred);
    g.setColour(juce::Colour(tokens::text));
    g.setFont(fonts::mono(static_cast<float>(tokens::fontSizeLabel)));
    g.drawText(formatPitch(static_cast<float>(pitch.getValue())), pitchValue,
               juce::Justification::centred, false);
}
