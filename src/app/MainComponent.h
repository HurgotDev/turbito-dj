#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <optional>

#include "engine/AudioEngine.h"
#include "library/Library.h"
#include "library/Query.h"
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
    void trackLoaded(int deck, const juce::File& file, const juce::String& name);
    void refreshLibrary();
    void importFolder();
    void showDeckRecord(int deck);
    void updateMasterBpm();
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
    float crossfader = 0.5f;

    // Null when the database could not be opened: the app runs without a library.
    // After the view, so it goes first and its callbacks never reach a dead view.
    std::unique_ptr<Library> library;
    juce::String searchText;
    LibraryFilter filter = LibraryFilter::all;
    int collection = 0;
    // Insertion (or history) order until the user clicks a column header.
    std::optional<std::pair<LibraryColumn, bool>> sortOrder;
    std::unique_ptr<juce::FileChooser> chooser;

    // Per deck: the loaded record (0 = none), its 50 ms peaks and the last bar index shown.
    std::array<juce::int64, AudioEngine::numDecks> loadedId {};
    std::array<std::vector<std::uint8_t>, AudioEngine::numDecks> peaks;
    std::array<int, AudioEngine::numDecks> shownBar {};
    juce::String masterBpm;

    juce::Component::SafePointer<juce::DialogWindow> settingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
