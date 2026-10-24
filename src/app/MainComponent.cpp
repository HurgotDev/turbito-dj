#include "app/MainComponent.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include "engine/deck/TrackLoader.h"
#include "ui/Strings.h"
#include "ui/look/Tokens.h"

namespace
{
    constexpr const char* audioDeviceKey = "audioDevice";

    juce::PropertiesFile::Options settingsOptions()
    {
        juce::PropertiesFile::Options options;
        options.applicationName = "Turbito DJ";
        options.filenameSuffix = "settings";
        options.osxLibrarySubFolder = "Application Support";
        return options;
    }

    juce::String formatTime(double seconds)
    {
        const auto tenths = juce::roundToInt(juce::jmax(0.0, seconds) * 10.0);
        return juce::String::formatted("%02d:%02d.%d", tenths / 600, (tenths / 10) % 60, tenths % 10);
    }

    juce::String formatLength(double seconds)
    {
        const auto whole = (int) juce::jmax(0.0, seconds);
        return juce::String::formatted("%02d:%02d", whole / 60, whole % 60);
    }
}

MainComponent::MainComponent()
    : settings(settingsOptions()),
      trackName(strings::noTrack)
{
    const auto savedState = settings.getXmlValue(audioDeviceKey);
    deviceManager.initialise(0, 2, savedState.get(), true);
    deviceManager.addAudioCallback(&engine);

    loadButton.setButtonText(strings::load);
    playButton.setButtonText(strings::play);
    cueButton.setButtonText(strings::cue);
    settingsButton.setButtonText(strings::audioSettings);

    loadButton.onClick = [this] { chooseFile(); };
    playButton.onClick = [this] { engine.post({ Action::DeckA_Play, 1.0f }); };
    cueButton.onStateChange = [this] { setCue(cueButton.isDown(), cueButtonDown); };
    settingsButton.onClick = [this] { openAudioSettings(); };

    for (auto* button : { &loadButton, &playButton, &cueButton, &settingsButton })
    {
        button->setWantsKeyboardFocus(false);
        addAndMakeVisible(button);
    }

    addAndMakeVisible(status);

    setWantsKeyboardFocus(true);
    setSize(tokens::windowReference.width, tokens::windowReference.height);
    startTimerHz(30);

    juce::MessageManager::callAsync([safe = juce::Component::SafePointer<MainComponent>(this)]
    {
        if (safe != nullptr)
            safe->grabKeyboardFocus();
    });
}

MainComponent::~MainComponent()
{
    stopTimer();
    delete settingsWindow.getComponent();

    if (const auto state = deviceManager.createStateXml())
        settings.setValue(audioDeviceKey, state.get());

    deviceManager.removeAudioCallback(&engine);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(tokens::bg));
}

void MainComponent::resized()
{
    auto row = getLocalBounds().reduced(16).removeFromTop(32);

    for (auto* button : { &loadButton, &playButton, &cueButton })
    {
        button->setBounds(row.removeFromLeft(96));
        row.removeFromLeft(8);
    }

    settingsButton.setBounds(row.removeFromLeft(160));
    row.removeFromLeft(16);
    status.setBounds(row);
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        engine.post({ Action::DeckA_Play, 1.0f });
        return true;
    }

    if (key.getKeyCode() == 'Q' || key.getKeyCode() == 'q')
    {
        setCue(true, cueKeyDown);
        return true;
    }

    return false;
}

bool MainComponent::keyStateChanged(bool)
{
    if (cueKeyDown && ! juce::KeyPress::isKeyCurrentlyDown('Q'))
    {
        setCue(false, cueKeyDown);
        return true;
    }

    return false;
}

void MainComponent::setCue(bool down, bool& state)
{
    // Key repeat and button repaints call this again without a transition.
    if (down == state)
        return;

    state = down;
    engine.post({ Action::DeckA_Cue, down ? 1.0f : 0.0f });
}

void MainComponent::chooseFile()
{
    chooser = std::make_unique<juce::FileChooser>(strings::chooseTrack, juce::File(),
                                                  "*.wav;*.aif;*.aiff;*.flac;*.mp3");

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                         [safe = juce::Component::SafePointer<MainComponent>(this)](const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();

        if (file == juce::File())
            return;

        juce::Thread::launch([safe, file]
        {
            // std::function needs a copyable lambda, so the track travels in a shared_ptr.
            auto track = std::make_shared<std::unique_ptr<Track>>(loadTrack(file));

            juce::MessageManager::callAsync([safe, track]
            {
                if (safe == nullptr)
                    return;

                if (*track == nullptr)
                {
                    safe->trackName = strings::loadFailed;
                    return;
                }

                const auto name = (*track)->name;
                safe->trackName = safe->engine.load(0, std::move(*track)) ? name : juce::String(strings::loadFailed);
            });
        });
    });
}

void MainComponent::openAudioSettings()
{
    if (settingsWindow != nullptr)
    {
        settingsWindow->toFront(true);
        return;
    }

    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(deviceManager, 0, 0, 2, 2,
                                                                          false, false, true, false);
    selector->setSize(500, 400);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(selector.release());
    options.dialogTitle = strings::audioSettings;
    options.dialogBackgroundColour = juce::Colour(tokens::bg);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    settingsWindow = options.launchAsync();
}

void MainComponent::timerCallback()
{
    const auto& snapshot = engine.snapshot().decks[0];
    auto text = trackName;

    if (snapshot.loaded.load())
        text << "   " << formatTime(snapshot.positionSeconds.load())
             << " / " << formatLength(snapshot.lengthSeconds.load());

    status.setText(text, juce::dontSendNotification);
    playButton.setButtonText(snapshot.playing.load() ? strings::pause : strings::play);
    engine.collectRetired();
}
