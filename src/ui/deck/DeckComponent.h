#pragma once

#include <array>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "core/Action.h"
#include "ui/widgets/JogWheel.h"
#include "ui/widgets/PadGrid.h"

// Pitch as the mockup shows it: "0.0%", "+1.2%", "-1.6%".
juce::String formatPitch(float percent);

// One deck panel: header, progress, jog, transport, loops, pitch and pads.
// Knows nothing of the engine: data comes in through setters, user input
// goes out through onAction.
class DeckComponent : public juce::Component
{
public:
    // 0 = deck A, 1 = deck B.
    explicit DeckComponent(int deck);

    void setTrack(const juce::String& title, const juce::String& artist);
    void setBpm(const juce::String& bpm);
    void setKey(const juce::String& key);
    void setPlaying(bool playing);
    void setPosition(double positionSeconds, double lengthSeconds);
    void setPitch(float percent);
    void setSync(bool on);

    PadGrid& pads();

    std::function<void(ActionEvent)> onAction;

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    void emit(Action action, float value);

    const int deck;
    const juce::Colour accent;

    juce::String title, artist, bpm, key;
    double position = 0.0;
    double length = 0.0;
    bool cueDown = false;

    JogWheel jog;
    juce::TextButton cue, play, sync, keylock;
    std::array<juce::TextButton, 6> loopSizes;
    std::array<juce::TextButton, 3> loopControls;
    juce::Slider pitch;
    PadGrid padGrid;

    juce::Rectangle<int> header, progress, pitchLabel, pitchValue;
};
