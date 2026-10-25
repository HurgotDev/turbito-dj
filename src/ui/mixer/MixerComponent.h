#pragma once

#include <array>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "core/Action.h"
#include "ui/widgets/Knob.h"
#include "ui/widgets/VuMeter.h"

// Mixer panel: two channels (gain, EQ, filter, VU, fader), the FX unit,
// Placas volume and the crossfader. Knows nothing of the engine: data comes
// in through setters, user input goes out through onAction.
class MixerComponent : public juce::Component
{
public:
    static constexpr int preferredHeight = 400;

    MixerComponent();

    // 0 = channel A, 1 = channel B; call at UI rate with the block peak.
    void setPeak(int deck, float peakLinear);

    // Moves the control that sends `action` to a 0–1 value, without sending.
    void setControl(Action action, float value);

    void setPlacasVolume(float volume);

    std::function<void(ActionEvent)> onAction;

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    struct Channel
    {
        std::array<Knob, 5> knobs { { { 26, 0.5 }, { 26, 0.5 }, { 26, 0.5 }, { 26, 0.5 },
                                      { 26, 0.5 } } };
        VuMeter vu;
        juce::Slider fader { juce::Slider::LinearVertical, juce::Slider::NoTextBox };
        juce::Rectangle<int> title;
        std::array<juce::Rectangle<int>, 5> knobLabels;
    };

    juce::Slider* sliderFor(Action action);

    std::array<Channel, 2> channels;

    juce::ComboBox fxSelect;
    Knob dryWet { 24, 0.4 };
    std::array<juce::TextButton, 4> fxBeats;
    std::array<juce::TextButton, 2> fxTargets;
    juce::Slider placas { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::Slider crossfader { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };

    juce::Rectangle<int> fxLabel, fxSeparator, placasSeparator, placasLabel, placasValue,
        letterA, letterB;
};
