#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

#include "engine/AudioEngine.h"

// shortcut: temporary phase 2 UI with plain JUCE widgets, replaced in phase 4
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
    void chooseFile();
    void openAudioSettings();
    void setCue(bool down, bool& state);

    juce::PropertiesFile settings;
    // Declared before the device manager so it outlives the device.
    AudioEngine engine;
    juce::AudioDeviceManager deviceManager;

    juce::TextButton loadButton, playButton, cueButton, settingsButton;
    juce::Label status;
    juce::String trackName;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::Component::SafePointer<juce::DialogWindow> settingsWindow;

    bool cueButtonDown = false;
    bool cueKeyDown = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
