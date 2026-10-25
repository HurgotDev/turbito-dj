#include "app/MainComponent.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include "engine/deck/TrackLoader.h"
#include "ui/Strings.h"
#include "ui/look/Tokens.h"

namespace
{
    constexpr const char* audioDeviceKey = "audioDevice";
    constexpr const char* crossfaderCurveKey = "crossfaderCurve";

    // Indexed by deck: 0 = A, 1 = B.
    constexpr Action playActions[] = { Action::DeckA_Play, Action::DeckB_Play };
    constexpr Action cueActions[] = { Action::DeckA_Cue, Action::DeckB_Cue };
    constexpr int cueKeys[] = { 'Q', 'P' };

    juce::PropertiesFile::Options settingsOptions()
    {
        juce::PropertiesFile::Options options;
        options.applicationName = "Turbito DJ";
        options.filenameSuffix = "settings";
        options.osxLibrarySubFolder = "Application Support";
        return options;
    }

    // Ajustes: the audio device selector and, below it, the crossfader curve.
    class SettingsPanel : public juce::Component
    {
    public:
        SettingsPanel(juce::AudioDeviceManager& deviceManager, float curve,
                      std::function<void(float)> onCurve)
            : selector(deviceManager, 0, 0, 2, 2, false, false, true, false)
        {
            curveLabel.setText(strings::crossfaderCurve, juce::dontSendNotification);
            curveLabel.setColour(juce::Label::textColourId, juce::Colour(tokens::text));
            curveSlider.setRange(0.0, 1.0);
            curveSlider.setValue(curve, juce::dontSendNotification);
            curveSlider.setDoubleClickReturnValue(true, 0.0);
            curveSlider.onValueChange = [this, onCurve] { onCurve((float) curveSlider.getValue()); };

            addAndMakeVisible(selector);
            addAndMakeVisible(curveLabel);
            addAndMakeVisible(curveSlider);
            setSize(500, 440);
        }

        void resized() override
        {
            auto area = getLocalBounds();
            auto curveRow = area.removeFromBottom(40).reduced(16, 8);
            selector.setBounds(area);
            curveLabel.setBounds(curveRow.removeFromLeft(160));
            curveSlider.setBounds(curveRow);
        }

    private:
        juce::AudioDeviceSelectorComponent selector;
        juce::Label curveLabel;
        juce::Slider curveSlider { juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    };
}

MainComponent::MainComponent()
    : settings(settingsOptions())
{
    const auto savedState = settings.getXmlValue(audioDeviceKey);
    deviceManager.initialise(0, 2, savedState.get(), true);
    deviceManager.addAudioCallback(&engine);
    deviceManager.addChangeListener(this);
    engine.post({ Action::Mixer_CrossfaderCurve, (float) settings.getDoubleValue(crossfaderCurveKey) });

    view.onAction = [this](ActionEvent event) { engine.post(event); };
    view.library().onLoad = [this](int deck, const juce::File& file) { loadFile(deck, file); };

    auto& top = view.topBar();
    top.setController(false, strings::noController);
    top.setMasterBpm(juce::String::fromUTF8(strings::noValue));
    top.onSettings = [this] { openSettings(); };
    changeListenerCallback(&deviceManager);

    addAndMakeVisible(view);
    setWantsKeyboardFocus(true);
    setSize(tokens::windowReference.width, tokens::windowReference.height);
    startTimerHz(60);

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

    deviceManager.removeChangeListener(this);
    deviceManager.removeAudioCallback(&engine);
}

void MainComponent::resized()
{
    view.setBounds(getLocalBounds());
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey || key == juce::KeyPress::returnKey)
    {
        engine.post({ playActions[key == juce::KeyPress::returnKey ? 1 : 0], 1.0f });
        return true;
    }

    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
    {
        if (juce::CharacterFunctions::toUpperCase((juce::juce_wchar) key.getKeyCode()) == cueKeys[deck])
        {
            setCue(deck, true);
            return true;
        }
    }

    return false;
}

bool MainComponent::keyStateChanged(bool)
{
    auto handled = false;

    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
    {
        if (cueKeyDown[deck] && ! juce::KeyPress::isKeyCurrentlyDown(cueKeys[deck]))
        {
            setCue(deck, false);
            handled = true;
        }
    }

    return handled;
}

void MainComponent::setCue(int deck, bool down)
{
    // Key repeat calls this again without a transition.
    if (down == cueKeyDown[deck])
        return;

    cueKeyDown[deck] = down;
    engine.post({ cueActions[deck], down ? 1.0f : 0.0f });
}

void MainComponent::setCrossfaderCurve(float curve)
{
    settings.setValue(crossfaderCurveKey, curve);
    engine.post({ Action::Mixer_CrossfaderCurve, curve });
}

void MainComponent::loadFile(int deck, const juce::File& file)
{
    juce::Thread::launch([safe = juce::Component::SafePointer<MainComponent>(this), file, deck]
    {
        // std::function needs a copyable lambda, so the track travels in a shared_ptr.
        auto track = std::make_shared<std::unique_ptr<Track>>(loadTrack(file));

        juce::MessageManager::callAsync([safe, track, file, deck]
        {
            if (safe == nullptr)
                return;

            const auto name = *track != nullptr ? (*track)->name : juce::String();

            if (*track == nullptr || ! safe->engine.load(deck, std::move(*track)))
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions::makeOptionsOk(
                                                 juce::MessageBoxIconType::WarningIcon,
                                                 strings::loadFailed, file.getFileName()),
                                             nullptr);
                return;
            }

            safe->view.deck(deck).setTrack(name, {});
            safe->view.library().setLoadedDeck(deck, file);
        });
    });
}

void MainComponent::openSettings()
{
    if (settingsWindow != nullptr)
    {
        settingsWindow->toFront(true);
        return;
    }

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(new SettingsPanel(deviceManager,
                                               (float) settings.getDoubleValue(crossfaderCurveKey),
                                               [this](float curve) { setCrossfaderCurve(curve); }));
    options.dialogTitle = strings::settings;
    options.dialogBackgroundColour = juce::Colour(tokens::bg);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    settingsWindow = options.launchAsync();
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster*)
{
    auto* device = deviceManager.getCurrentAudioDevice();
    view.topBar().setAudio(device != nullptr ? device->getCurrentSampleRate() : 0.0,
                           device != nullptr ? device->getCurrentBufferSizeSamples() : 0);
}

void MainComponent::timerCallback()
{
    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
    {
        const auto& snapshot = engine.snapshot().decks[deck];
        auto& deckView = view.deck(deck);
        deckView.setPlaying(snapshot.playing.load());
        deckView.setPosition(snapshot.positionSeconds.load(), snapshot.lengthSeconds.load());
        view.mixer().setPeak(deck, snapshot.channelPeak.load());
    }

    engine.collectRetired();
}
