#include "ui/topbar/TopBar.h"

#include "BinaryData.h"
#include "ui/Layout.h"
#include "ui/Strings.h"
#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"
#include "ui/look/TurbitoLookAndFeel.h"

namespace
{
    constexpr int insetX = 1 + 14;  // border + padding
    constexpr int insetY = 1 + 7;
    constexpr int logoWidth = 168;
    constexpr int logoHeight = 36;
    constexpr int groupGap = 16;
    constexpr int dotSize = 8;
    constexpr int dotGap = 8;
    constexpr int masterGap = 6;
    constexpr int buttonHeight = 34;
    constexpr int buttonPadding = 12;
    constexpr int recordDot = 10;
    constexpr int recordGap = 6;
    constexpr int iconSize = 16;

    juce::Font statusFont() { return fonts::ui(static_cast<float>(tokens::fontSizeButton)); }
    juce::Font recordFont() { return fonts::ui(static_cast<float>(tokens::fontSizeButton), 600); }
    juce::Font bpmFont() { return fonts::mono(16.0f, 700); }

    juce::Font masterLabelFont()
    {
        return fonts::tracked(fonts::ui(static_cast<float>(tokens::fontSizeLabel)),
                              tokens::labelTracking);
    }

    juce::String masterLabel() { return upperCase(strings::master); }

    // Feather's "settings" icon from the mockup, in its 24 × 24 viewBox.
    juce::Path gearPath()
    {
        auto path = juce::Drawable::parseSVGPath(
            "M19.4 15a1.7 1.7 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.8-.3 "
            "1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1.1-1.5 1.7 1.7 0 0 0-1.8.3l-.1.1"
            "a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1"
            "a1.7 1.7 0 0 0 1.5-1.1 1.7 1.7 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 "
            "1.8.3H9a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 "
            "1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.8V9a1.7 1.7 0 0 0 1.5 1H21"
            "a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1z");
        path.addEllipse(9.0f, 9.0f, 6.0f, 6.0f);
        return path;
    }
}

juce::String formatAudioStatus(double sampleRate, int bufferSize)
{
    if (sampleRate <= 0.0)
        return juce::String::fromUTF8(strings::noAudioDevice);

    auto khz = juce::String(sampleRate / 1000.0, 1);

    if (khz.endsWith(".0"))
        khz = khz.dropLastCharacters(2);

    return strings::audioPrefix + khz + juce::String::fromUTF8(strings::audioBuffer)
         + juce::String(bufferSize);
}

TopBar::PaintedButton::PaintedButton(const juce::String& name) : juce::Button(name)
{
    setButtonText(name);
}

void TopBar::PaintedButton::paintButton(juce::Graphics& g, bool isHighlighted, bool isDown)
{
    getLookAndFeel().drawButtonBackground(g, *this, findColour(juce::TextButton::buttonColourId),
                                          isHighlighted, isDown);
    if (paintContent)
        paintContent(g);
}

TopBar::TopBar()
    : controller(strings::noController),
      audio(formatAudioStatus(44100.0, 256)),
      masterBpm(juce::String::fromUTF8(strings::noValue)),
      record(strings::record),
      settings(strings::settings)
{
    logo = juce::Drawable::createFromSVGString(juce::String::fromUTF8(
        BinaryData::turbitohorizontal_svg, BinaryData::turbitohorizontal_svgSize));

    record.setColour(TurbitoLookAndFeel::buttonBorderColourId, juce::Colour(tokens::recordBorder));
    record.paintContent = [this](juce::Graphics& g)
    {
        auto area = record.getLocalBounds().reduced(1 + buttonPadding, 0);  // border + padding
        const auto dot = area.removeFromLeft(recordDot).toFloat();
        g.setColour(juce::Colour(tokens::record));
        g.fillEllipse(juce::Rectangle<float>(8.0f, 8.0f).withCentre(dot.getCentre()));

        area.removeFromLeft(recordGap);
        g.setColour(juce::Colour(tokens::recordText));
        g.setFont(recordFont());
        g.drawText(record.getButtonText(), area, juce::Justification::centredLeft, false);
    };
    record.onClick = [this] { if (onRecord) onRecord(); };

    settings.setColour(TurbitoLookAndFeel::buttonBorderColourId, juce::Colour(tokens::border));
    settings.setTooltip(strings::settings);
    settings.paintContent = [this](juce::Graphics& g)
    {
        static const auto gear = gearPath();
        const auto box = juce::Rectangle<float>(static_cast<float>(iconSize),
                                                static_cast<float>(iconSize))
                             .withCentre(settings.getLocalBounds().toFloat().getCentre());
        g.setColour(juce::Colour(tokens::text));
        // JUCE strokes after transforming, so scale the viewBox's 2px stroke by hand.
        g.strokePath(gear, juce::PathStrokeType(2.0f * box.getWidth() / 24.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded),
                     juce::AffineTransform::scale(box.getWidth() / 24.0f)
                         .translated(box.getX(), box.getY()));
    };
    settings.onClick = [this] { if (onSettings) onSettings(); };

    for (auto* button : { &record, &settings })
    {
        button->setWantsKeyboardFocus(false);
        addAndMakeVisible(button);
    }
}

void TopBar::setController(bool connected, const juce::String& name)
{
    controllerConnected = connected;
    controller = name;
    resized();
    repaint();
}

void TopBar::setAudio(double sampleRate, int bufferSize)
{
    audio = formatAudioStatus(sampleRate, bufferSize);
    resized();
    repaint();
}

void TopBar::setMasterBpm(const juce::String& bpm)
{
    masterBpm = bpm;
    resized();
    repaint();
}

void TopBar::resized()
{
    auto area = getLocalBounds().reduced(insetX, insetY);
    logoArea = area.removeFromLeft(logoWidth).withSizeKeepingCentre(logoWidth, logoHeight);

    // Right group, laid out from the right edge.
    const auto centred = [&area](int width, int height)
    {
        auto slot = area.removeFromRight(width).withSizeKeepingCentre(width, height);
        area.removeFromRight(groupGap);
        return slot;
    };

    settings.setBounds(centred(buttonHeight, buttonHeight));
    record.setBounds(centred(2 * buttonPadding + recordDot + recordGap
                                 + textWidth(recordFont(), record.getButtonText()) + 2,
                             buttonHeight));

    const auto masterWidth = textWidth(masterLabelFont(), masterLabel()) + masterGap
                           + textWidth(bpmFont(), masterBpm);
    masterArea = centred(masterWidth, area.getHeight());
    audioArea = centred(textWidth(statusFont(), audio), area.getHeight());
    controllerArea = centred(dotSize + dotGap + textWidth(statusFont(), controller),
                             area.getHeight());
}

void TopBar::paint(juce::Graphics& g)
{
    TurbitoLookAndFeel::drawPanel(g, getLocalBounds().toFloat());

    // Fit the SVG's own 520 × 112 canvas, as a browser does, not its painted bounds.
    if (logo != nullptr)
        logo->draw(g, 1.0f, juce::RectanglePlacement(juce::RectanglePlacement::centred)
                                .getTransformToFit({ 0.0f, 0.0f, 520.0f, 112.0f }, logoArea.toFloat()));

    auto controllerText = controllerArea;
    const auto dot = controllerText.removeFromLeft(dotSize).toFloat();
    g.setColour(juce::Colour(controllerConnected ? tokens::vuLow : tokens::empty));
    g.fillEllipse(juce::Rectangle<float>(static_cast<float>(dotSize), static_cast<float>(dotSize))
                      .withCentre(dot.getCentre()));
    controllerText.removeFromLeft(dotGap);

    g.setColour(juce::Colour(tokens::textMuted));
    g.setFont(statusFont());
    g.drawText(controller, controllerText, juce::Justification::centredLeft, false);
    g.drawText(audio, audioArea, juce::Justification::centredLeft, false);

    // "MASTER" and the BPM share a baseline, the BPM line centred.
    const auto bpm = bpmFont();
    const auto baseline = static_cast<float>(masterArea.getCentreY())
                        - bpm.getHeight() / 2 + bpm.getAscent();
    const auto label = masterLabelFont();
    const auto bpmX = masterArea.getX() + textWidth(label, masterLabel()) + masterGap;

    g.setFont(label);
    g.drawSingleLineText(masterLabel(), masterArea.getX(), juce::roundToInt(baseline));
    g.setColour(juce::Colour(tokens::text));
    g.setFont(bpm);
    g.drawSingleLineText(masterBpm, bpmX, juce::roundToInt(baseline));
}
