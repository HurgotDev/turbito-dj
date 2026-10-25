#include "ui/widgets/JogWheel.h"

#include <cmath>

#include "ui/look/Fonts.h"
#include "ui/look/Tokens.h"

namespace
{
    constexpr float ringWidth = 6.0f;
    constexpr float markerWidth = 4.0f;
    constexpr float markerLength = 18.0f;
    constexpr float markerTop = ringWidth + 5.0f;  // the mockup's 5 is inside the ring
    constexpr double secondsPerTurn = 1.8;
}

float jogAngleDegrees(double positionSeconds)
{
    const auto degrees = std::fmod(positionSeconds * 360.0 / secondsPerTurn, 360.0);
    return static_cast<float>(std::fmod(degrees + 360.0, 360.0));
}

juce::String formatElapsed(double seconds)
{
    const auto whole = static_cast<int>(std::floor(juce::jmax(0.0, seconds)));
    return juce::String::formatted("%d:%02d", whole / 60, whole % 60);
}

juce::String formatRemaining(double positionSeconds, double lengthSeconds)
{
    return "-" + formatElapsed(lengthSeconds - positionSeconds);
}

JogWheel::JogWheel()
    : accent(tokens::deckA)
{
    setSize(tokens::layoutJogSize, tokens::layoutJogSize);
}

void JogWheel::setAccent(juce::Colour newAccent)
{
    accent = newAccent;
    repaint();
}

void JogWheel::setPosition(double positionSeconds, double lengthSeconds)
{
    position = positionSeconds;
    length = lengthSeconds;
    repaint();
}

void JogWheel::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto centre = bounds.getCentre();

    g.setColour(juce::Colour(tokens::bg));
    g.fillEllipse(bounds);

    g.setColour(juce::Colour(tokens::empty));
    g.drawEllipse(bounds.reduced(ringWidth / 2.0f), ringWidth);

    const juce::Rectangle<float> marker(centre.x - markerWidth / 2.0f, markerTop, markerWidth,
                                        markerLength);
    juce::Path markerPath;
    markerPath.addRoundedRectangle(marker, markerWidth / 2.0f);

    const auto turn = juce::AffineTransform::rotation(
        juce::degreesToRadians(jogAngleDegrees(position)), centre.x, centre.y);
    g.setColour(accent);
    g.fillPath(markerPath, turn);

    const auto elapsedFont = fonts::mono(static_cast<float>(tokens::fontSizeJogTime), 700);
    const auto remainingFont = fonts::mono(static_cast<float>(tokens::fontSizeLabel), 400);
    const auto blockHeight = elapsedFont.getHeight() + remainingFont.getHeight();
    auto text = bounds.withSizeKeepingCentre(bounds.getWidth(), blockHeight);

    g.setColour(juce::Colour(tokens::text));
    g.setFont(elapsedFont);
    g.drawText(formatElapsed(position), text.removeFromTop(elapsedFont.getHeight()),
               juce::Justification::centred, false);

    g.setColour(juce::Colour(tokens::textMuted));
    g.setFont(remainingFont);
    g.drawText(formatRemaining(position, length), text, juce::Justification::centred, false);
}
