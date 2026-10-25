#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "engine/AudioEngine.h"
#include "ui/AppView.h"
#include "ui/look/TurbitoLookAndFeel.h"

// Connects the engine, the audio device and the settings to the AppView.
class MainComponent : public juce::Component,
                      private juce::Timer,
                      private juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

private:
    // The default LookAndFeel from before any component is built until after
    // all of them are gone.
    struct DefaultLookAndFeel
    {
        DefaultLookAndFeel() { juce::LookAndFeel::setDefaultLookAndFeel(&look); }
        ~DefaultLookAndFeel() { juce::LookAndFeel::setDefaultLookAndFeel(nullptr); }
        TurbitoLookAndFeel look;
    };

    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void loadFile(int deck, const juce::File& file);
    void openSettings();
    void setCue(int deck, bool down);
    void setCrossfaderCurve(float curve);

    DefaultLookAndFeel lookAndFeel;
    juce::PropertiesFile settings;
    // Declared before the device manager so it outlives the device.
    AudioEngine engine;
    juce::AudioDeviceManager deviceManager;

    AppView view;
    bool cueKeyDown[AudioEngine::numDecks] {};

    juce::Component::SafePointer<juce::DialogWindow> settingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
