#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

#include "engine/AudioEngine.h"

// shortcut: temporary phase 2-3 UI with plain JUCE widgets, replaced in phase 4
class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

private:
    void timerCallback() override;
    void chooseFile(int deck);
    void openAudioSettings();
    void setCue(int deck, bool down, bool& state);
    void setupSlider(juce::Slider& slider, Action action, double defaultValue);
    void paintVu(juce::Graphics& g, int deck) const;

    struct DeckRow
    {
        juce::Label name;
        juce::TextButton load, play, cue;
        juce::Label status;
        juce::String trackName;
        bool cueButtonDown = false;
        bool cueKeyDown = false;
    };

    struct ChannelStrip
    {
        static constexpr int numKnobs = 5;

        juce::Label name;
        juce::Slider knobs[numKnobs];
        juce::Label knobLabels[numKnobs];
        juce::Slider fader;
        juce::Rectangle<int> vuBounds;
        float shownPeak = 0.0f;
    };

    juce::PropertiesFile settings;
    // Declared before the device manager so it outlives the device.
    AudioEngine engine;
    juce::AudioDeviceManager deviceManager;

    DeckRow deckRows[AudioEngine::numDecks];
    ChannelStrip strips[AudioEngine::numDecks];
    juce::Slider crossfader, crossfaderCurve;
    juce::Label crossfaderLabel, curveLabel;
    juce::TextButton settingsButton;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::Component::SafePointer<juce::DialogWindow> settingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
