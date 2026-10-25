#include "ui/widgets/WaveformView.h"

#include <cmath>

#include "ui/look/Tokens.h"

namespace
{
    constexpr float radius = 6.0f;
    constexpr float padding = 2.0f;
    constexpr float barGap = 1.0f;
    constexpr float barRadius = 1.0f;
    constexpr float minBarHeight = 2.0f;
    constexpr int placeholderBars = 160;
    constexpr float loopFillAlpha = 0.16f;
    constexpr float playheadWidth = 2.0f;
}

std::vector<float> barsAround(const std::vector<std::uint8_t>& peaks, double positionSeconds,
                              int count)
{
    constexpr double peaksPerSecond = 20.0;
    constexpr float maxBarHeight = 48.0f;

    std::vector<float> heights(static_cast<std::size_t>(juce::jmax(0, count)), 0.0f);
    const auto first = static_cast<juce::int64>(std::llround(positionSeconds * peaksPerSecond)) - count / 2;

    for (std::size_t i = 0; i < heights.size(); ++i)
        if (const auto index = first + static_cast<juce::int64>(i);
            index >= 0 && index < static_cast<juce::int64>(peaks.size()))
            heights[i] = peaks[static_cast<std::size_t>(index)] / 255.0f * maxBarHeight;

    return heights;
}

WaveformView::WaveformView()
    : accent(tokens::deckA),
      played(tokens::waveformPlayedA)
{
}

void WaveformView::setColours(juce::Colour newAccent, juce::Colour newPlayed)
{
    accent = newAccent;
    played = newPlayed;
    repaint();
}

void WaveformView::setBars(std::vector<float> heights)
{
    bars = std::move(heights);
    repaint();
}

void WaveformView::setLoop(std::optional<juce::Range<float>> fractions)
{
    loop = fractions;
    repaint();
}

void WaveformView::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto centreX = bounds.getCentreX();

    g.setColour(juce::Colour(tokens::bg));
    g.fillRoundedRectangle(bounds, radius);

    juce::Path clip;
    clip.addRoundedRectangle(bounds, radius);

    g.saveState();
    g.reduceClipRegion(clip);

    const auto inner = bounds.reduced(padding, 0.0f);
    const auto count = bars.empty() ? placeholderBars : static_cast<int>(bars.size());
    const auto barWidth = (inner.getWidth() - barGap * static_cast<float>(count - 1))
                        / static_cast<float>(count);

    for (int i = 0; i < count; ++i)
    {
        const auto x = inner.getX() + static_cast<float>(i) * (barWidth + barGap);
        const auto height = bars.empty() ? minBarHeight
                                         : juce::jmax(minBarHeight, bars[static_cast<size_t>(i)]);

        const auto silent = bars.empty() || bars[static_cast<size_t>(i)] <= 0.0f;
        g.setColour(silent                           ? juce::Colour(tokens::empty)
                    : x + barWidth / 2.0f < centreX  ? played
                                                     : accent);
        g.fillRoundedRectangle(juce::Rectangle<float>(x, bounds.getCentreY() - height / 2.0f,
                                                      barWidth, height),
                               barRadius);
    }

    if (loop)
    {
        const auto left = bounds.getX() + bounds.getWidth() * loop->getStart();
        // The mockup's 1px borders sit outside the loop's width, as CSS content-box.
        const auto right = bounds.getX() + bounds.getWidth() * loop->getEnd() + 2.0f;
        const juce::Rectangle<float> region(left, bounds.getY(), right - left, bounds.getHeight());

        g.setColour(accent.withAlpha(loopFillAlpha));
        g.fillRect(region);

        g.setColour(accent);
        g.fillRect(region.withWidth(1.0f));
        g.fillRect(region.withLeft(region.getRight() - 1.0f).withWidth(1.0f));
    }

    g.restoreState();

    g.setColour(juce::Colour(tokens::playhead));
    g.fillRect(juce::Rectangle<float>(centreX - playheadWidth / 2.0f, bounds.getY(),
                                      playheadWidth, bounds.getHeight()));
}
