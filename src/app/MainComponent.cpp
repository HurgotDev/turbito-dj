#include "app/MainComponent.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include "engine/deck/TrackLoader.h"
#include "ui/Strings.h"
#include "ui/look/Tokens.h"

namespace
{
    constexpr const char* audioDeviceKey = "audioDevice";

    // Indexed by deck: 0 = A, 1 = B.
    constexpr Action playActions[] = { Action::DeckA_Play, Action::DeckB_Play };
    constexpr Action cueActions[] = { Action::DeckA_Cue, Action::DeckB_Cue };
    constexpr int cueKeys[] = { 'Q', 'P' };
    constexpr Action faderActions[] = { Action::ChannelA_Fader, Action::ChannelB_Fader };

    constexpr Action knobActions[][5] = {
        { Action::ChannelA_Gain, Action::ChannelA_EqHigh, Action::ChannelA_EqMid,
          Action::ChannelA_EqLow, Action::ChannelA_Filter },
        { Action::ChannelB_Gain, Action::ChannelB_EqHigh, Action::ChannelB_EqMid,
          Action::ChannelB_EqLow, Action::ChannelB_Filter }
    };

    constexpr const char* knobNames[] = { strings::gain, strings::eqHigh, strings::eqMid,
                                          strings::eqLow, strings::filter };

    constexpr float vuFloorDb = -48.0f;

    float vuProportion(float db)
    {
        return juce::jmap(juce::jlimit(vuFloorDb, 0.0f, db), vuFloorDb, 0.0f, 0.0f, 1.0f);
    }

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
    : settings(settingsOptions())
{
    const auto savedState = settings.getXmlValue(audioDeviceKey);
    deviceManager.initialise(0, 2, savedState.get(), true);
    deviceManager.addAudioCallback(&engine);

    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
    {
        auto& row = deckRows[deck];
        row.trackName = strings::noTrack;
        row.name.setText(strings::deckNames[deck], juce::dontSendNotification);
        row.load.setButtonText(strings::load);
        row.play.setButtonText(strings::play);
        row.cue.setButtonText(strings::cue);

        row.load.onClick = [this, deck] { chooseFile(deck); };
        row.play.onClick = [this, deck] { engine.post({ playActions[deck], 1.0f }); };
        row.cue.onStateChange = [this, deck]
        {
            auto& r = deckRows[deck];
            setCue(deck, r.cue.isDown(), r.cueButtonDown);
        };

        for (auto* button : { &row.load, &row.play, &row.cue })
        {
            button->setWantsKeyboardFocus(false);
            addAndMakeVisible(button);
        }

        addAndMakeVisible(row.name);
        addAndMakeVisible(row.status);

        auto& strip = strips[deck];
        strip.name.setText(strings::deckNames[deck], juce::dontSendNotification);
        addAndMakeVisible(strip.name);

        for (int k = 0; k < ChannelStrip::numKnobs; ++k)
        {
            strip.knobs[k].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            setupSlider(strip.knobs[k], knobActions[deck][k], 0.5);
            strip.knobLabels[k].setText(knobNames[k], juce::dontSendNotification);
            strip.knobLabels[k].setJustificationType(juce::Justification::centred);
            addAndMakeVisible(strip.knobLabels[k]);
        }

        strip.fader.setSliderStyle(juce::Slider::LinearVertical);
        setupSlider(strip.fader, faderActions[deck], 1.0);
    }

    crossfader.setSliderStyle(juce::Slider::LinearHorizontal);
    setupSlider(crossfader, Action::Mixer_Crossfader, 0.5);
    crossfaderCurve.setSliderStyle(juce::Slider::LinearHorizontal);
    setupSlider(crossfaderCurve, Action::Mixer_CrossfaderCurve, 0.0);

    crossfaderLabel.setText(strings::crossfader, juce::dontSendNotification);
    crossfaderLabel.setJustificationType(juce::Justification::centred);
    curveLabel.setText(strings::crossfaderCurve, juce::dontSendNotification);
    addAndMakeVisible(crossfaderLabel);
    addAndMakeVisible(curveLabel);

    settingsButton.setButtonText(strings::audioSettings);
    settingsButton.onClick = [this] { openAudioSettings(); };
    settingsButton.setWantsKeyboardFocus(false);
    addAndMakeVisible(settingsButton);

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

    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
        paintVu(g, deck);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16);

    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
    {
        auto& row = deckRows[deck];
        auto bounds = area.removeFromTop(32);
        area.removeFromTop(8);

        if (deck == 0)
            settingsButton.setBounds(bounds.removeFromRight(160));

        row.name.setBounds(bounds.removeFromLeft(72));

        for (auto* button : { &row.load, &row.play, &row.cue })
        {
            button->setBounds(bounds.removeFromLeft(96));
            bounds.removeFromLeft(8);
        }

        bounds.removeFromLeft(8);
        row.status.setBounds(bounds);
    }

    area.removeFromTop(24);
    auto mixer = area.removeFromTop(360);

    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
    {
        auto& strip = strips[deck];
        auto bounds = deck == 0 ? mixer.removeFromLeft(mixer.getWidth() / 2) : mixer;
        strip.name.setBounds(bounds.removeFromTop(24));

        auto labels = bounds.removeFromTop(20);
        auto knobs = bounds.removeFromTop(80);

        for (int k = 0; k < ChannelStrip::numKnobs; ++k)
        {
            strip.knobLabels[k].setBounds(labels.removeFromLeft(88));
            strip.knobs[k].setBounds(knobs.removeFromLeft(88).reduced(4));
        }

        bounds.removeFromTop(16);
        auto faderColumn = bounds.removeFromTop(200);
        strip.fader.setBounds(faderColumn.removeFromLeft(48));
        faderColumn.removeFromLeft(8);
        strip.vuBounds = faderColumn.removeFromLeft(16);
    }

    crossfaderLabel.setBounds(area.removeFromTop(24).withSizeKeepingCentre(480, 24));
    crossfader.setBounds(area.removeFromTop(32).withSizeKeepingCentre(480, 32));
    area.removeFromTop(8);

    auto curveRow = area.removeFromTop(24).withSizeKeepingCentre(240, 24);
    curveLabel.setBounds(curveRow.removeFromLeft(64));
    crossfaderCurve.setBounds(curveRow);
}

void MainComponent::paintVu(juce::Graphics& g, int deck) const
{
    const auto bounds = strips[deck].vuBounds.toFloat();
    g.setColour(juce::Colour(tokens::surface));
    g.fillRect(bounds);

    const auto level = vuProportion(juce::Decibels::gainToDecibels(strips[deck].shownPeak, vuFloorDb));
    const struct { float fromDb, toDb; std::uint32_t colour; } zones[] = {
        { vuFloorDb, -12.0f, tokens::vuLow },
        { -12.0f, -3.0f, tokens::vuMid },
        { -3.0f, 0.0f, tokens::vuHigh }
    };

    for (const auto& zone : zones)
    {
        const auto bottom = vuProportion(zone.fromDb);
        const auto top = juce::jmin(level, vuProportion(zone.toDb));

        if (top <= bottom)
            break;

        const auto y = [&](float proportion) { return bounds.getBottom() - proportion * bounds.getHeight(); };
        g.setColour(juce::Colour(zone.colour));
        g.fillRect(juce::Rectangle<float>::leftTopRightBottom(bounds.getX(), y(top), bounds.getRight(), y(bottom)));
    }
}

void MainComponent::setupSlider(juce::Slider& slider, Action action, double defaultValue)
{
    slider.setRange(0.0, 1.0);
    slider.setValue(defaultValue, juce::dontSendNotification);
    slider.setDoubleClickReturnValue(true, defaultValue);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setWantsKeyboardFocus(false);
    slider.onValueChange = [this, &slider, action] { engine.post({ action, (float) slider.getValue() }); };
    addAndMakeVisible(slider);
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
            setCue(deck, true, deckRows[deck].cueKeyDown);
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
        if (deckRows[deck].cueKeyDown && ! juce::KeyPress::isKeyCurrentlyDown(cueKeys[deck]))
        {
            setCue(deck, false, deckRows[deck].cueKeyDown);
            handled = true;
        }
    }

    return handled;
}

void MainComponent::setCue(int deck, bool down, bool& state)
{
    // Key repeat and button repaints call this again without a transition.
    if (down == state)
        return;

    state = down;
    engine.post({ cueActions[deck], down ? 1.0f : 0.0f });
}

void MainComponent::chooseFile(int deck)
{
    chooser = std::make_unique<juce::FileChooser>(strings::chooseTrack, juce::File(),
                                                  "*.wav;*.aif;*.aiff;*.flac;*.mp3");

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                         [safe = juce::Component::SafePointer<MainComponent>(this), deck](const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();

        if (file == juce::File())
            return;

        juce::Thread::launch([safe, file, deck]
        {
            // std::function needs a copyable lambda, so the track travels in a shared_ptr.
            auto track = std::make_shared<std::unique_ptr<Track>>(loadTrack(file));

            juce::MessageManager::callAsync([safe, track, deck]
            {
                if (safe == nullptr)
                    return;

                if (*track == nullptr)
                {
                    safe->deckRows[deck].trackName = strings::loadFailed;
                    return;
                }

                const auto name = (*track)->name;
                safe->deckRows[deck].trackName = safe->engine.load(deck, std::move(*track))
                                                     ? name : juce::String(strings::loadFailed);
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
    for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
    {
        const auto& snapshot = engine.snapshot().decks[deck];
        auto& row = deckRows[deck];
        auto text = row.trackName;

        if (snapshot.loaded.load())
            text << "   " << formatTime(snapshot.positionSeconds.load())
                 << " / " << formatLength(snapshot.lengthSeconds.load());

        row.status.setText(text, juce::dontSendNotification);
        row.play.setButtonText(snapshot.playing.load() ? strings::pause : strings::play);

        // The engine reports one block's peak; decaying here makes the bar fall smoothly.
        auto& strip = strips[deck];
        strip.shownPeak = juce::jmax(snapshot.channelPeak.load(), strip.shownPeak * 0.85f);
        repaint(strip.vuBounds);
    }

    engine.collectRetired();
}
