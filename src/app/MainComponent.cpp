#include "app/MainComponent.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include "engine/deck/TrackLoader.h"
#include "ui/Strings.h"
#include "ui/widgets/JogWheel.h"
#include "ui/widgets/WaveformView.h"
#include "ui/look/Tokens.h"

namespace
{
    constexpr const char* audioDeviceKey = "audioDevice";
    constexpr const char* crossfaderCurveKey = "crossfaderCurve";

    // Indexed by deck: 0 = A, 1 = B.
    constexpr Action playActions[] = { Action::DeckA_Play, Action::DeckB_Play };
    constexpr Action cueActions[] = { Action::DeckA_Cue, Action::DeckB_Cue };
    constexpr int cueKeys[] = { 'Q', 'P' };

    // Next to the settings file: ~/Library/Application Support/Turbito DJ on macOS.
    juce::File databaseFile()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
#if JUCE_MAC
        dir = dir.getChildFile("Application Support");
#endif
        return dir.getChildFile("Turbito DJ/library.db");
    }

    juce::String noValue() { return juce::String::fromUTF8(strings::noValue); }

    // The table shows whole BPM ("124"), the deck and the top bar one decimal ("124.0").
    juce::String formatBpm(std::optional<double> bpm, bool wholeNumber)
    {
        if (! bpm)
            return noValue();
        return wholeNumber ? juce::String(juce::roundToInt(*bpm)) : juce::String(*bpm, 1);
    }

    juce::String orNoValue(const juce::String& text) { return text.isNotEmpty() ? text : noValue(); }

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

    view.onAction = [this](ActionEvent event)
    {
        if (event.action == Action::Mixer_Crossfader)
            crossfader = event.value;
        engine.post(event);
    };

    auto& libraryView = view.library();
    libraryView.onLoad = [this](int deck, const juce::File& file) { loadFile(deck, file); };
    libraryView.onSearchChanged = [this](const juce::String& text) { searchText = text; refreshLibrary(); };
    libraryView.onFilterChanged = [this](int index) { filter = static_cast<LibraryFilter>(index); refreshLibrary(); };
    libraryView.onCollectionSelected = [this](int index) { collection = index; refreshLibrary(); };
    libraryView.onSortChanged = [this](int columnId, bool ascending)
    {
        // Column ids run title..duration in LibraryColumn's order; anything else clears the sort.
        if (columnId >= LibraryComponent::titleColumn && columnId <= LibraryComponent::durationColumn)
            sortOrder = std::pair { static_cast<LibraryColumn>(columnId - LibraryComponent::titleColumn), ascending };
        else
            sortOrder.reset();
        refreshLibrary();
    };
    libraryView.onImport = [this] { importFolder(); };
    libraryView.onAnalyze = [this]
    {
        if (library != nullptr)
            library->analyzePending();
    };

    try
    {
        library = std::make_unique<Library>(databaseFile());
        library->onChanged = [this]
        {
            for (int deck = 0; deck < AudioEngine::numDecks; ++deck)
                if (loadedId[(size_t) deck] != 0 && peaks[(size_t) deck].empty())
                    showDeckRecord(deck); // analysed after it was loaded
            refreshLibrary();
        };
        // Resumes the analysis a previous session left half done.
        library->analyzePending();
    }
    catch (const std::exception& e)
    {
        juce::AlertWindow::showAsync(juce::MessageBoxOptions::makeOptionsOk(juce::MessageBoxIconType::WarningIcon,
                                                                            strings::libraryFailed, e.what()),
                                     nullptr);
    }
    refreshLibrary();

    auto& top = view.topBar();
    top.setController(false, strings::noController);
    updateMasterBpm();
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

            safe->trackLoaded(deck, file, name);
        });
    });
}

void MainComponent::trackLoaded(int deck, const juce::File& file, const juce::String& name)
{
    const auto* record = library != nullptr ? library->find(file) : nullptr;
    const auto d = static_cast<std::size_t>(deck);

    view.deck(deck).setTrack(record != nullptr ? record->title : name,
                             orNoValue(record != nullptr ? record->artist : juce::String()));
    loadedId[d] = record != nullptr ? record->id : 0;
    peaks[d].clear();
    showDeckRecord(deck);

    if (peaks[d].empty())
        view.waveforms().waveform(deck).setBars({});

    if (record != nullptr)
        library->addHistory(record->id);

    view.library().setLoadedDeck(deck, file);
    refreshLibrary();
}

void MainComponent::showDeckRecord(int deck)
{
    const auto d = static_cast<std::size_t>(deck);
    const auto* record = library != nullptr ? library->find(loadedId[d]) : nullptr;

    view.deck(deck).setBpm(formatBpm(record != nullptr ? record->bpm : std::nullopt, false));
    view.deck(deck).setKey(orNoValue(record != nullptr ? record->camelot : juce::String()));

    if (record != nullptr && record->analyzed)
    {
        peaks[d] = library->waveform(record->id);
        shownBar[d] = -1; // the timer draws it
    }
}

void MainComponent::refreshLibrary()
{
    if (library == nullptr)
        return;

    std::vector<LibraryRecord> rows;

    // Todas las pistas and Carpetas show every track (all come from a folder); Placas y drops
    // stays empty until phase 9.
    if (collection == 0 || collection == 1)
        rows = library->records();
    else if (collection == 2)
        for (const auto id : library->history())
            if (const auto* record = library->find(id))
                rows.push_back(*record);

    const auto* deckA = library->find(loadedId[0]);
    const auto deckABpm = deckA != nullptr ? deckA->bpm : std::nullopt;
    const auto deckAKey = deckA != nullptr ? deckA->camelot : juce::String();

    std::erase_if(rows, [&](const LibraryRecord& r)
    {
        return ! matchesSearch(r, searchText) || ! passesFilter(r, filter, deckABpm, deckAKey);
    });

    if (sortOrder)
        sortRecords(rows, sortOrder->first, sortOrder->second);

    std::vector<LibraryTrack> tracks;
    tracks.reserve(rows.size());
    for (const auto& r : rows)
        tracks.push_back({ r.file, r.title, orNoValue(r.artist), formatBpm(r.bpm, true), orNoValue(r.camelot),
                           formatElapsed(r.duration), r.id });

    auto& libraryView = view.library();
    libraryView.setTracks(std::move(tracks));
    // The mockup leaves Historial without a count.
    libraryView.setCollections({ { strings::collections[0], formatCount((int) library->records().size()) },
                                 { strings::collections[1], formatCount(library->folderCount()) },
                                 { strings::collections[2], {} },
                                 { strings::collections[3], formatCount(0) } });
}

void MainComponent::importFolder()
{
    if (library == nullptr)
        return;

    chooser = std::make_unique<juce::FileChooser>(strings::importTitle);
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                         [this](const juce::FileChooser& fc)
                         {
                             if (const auto folder = fc.getResult(); folder.isDirectory())
                                 library->importFolder(folder);
                         });
}

void MainComponent::updateMasterBpm()
{
    const auto& decks = engine.snapshot().decks;
    const bool playing[] = { decks[0].playing.load(), decks[1].playing.load() };
    // Both playing: the deck the crossfader favours, A at the centre.
    const auto deck = playing[0] && playing[1] ? (crossfader > 0.5f ? 1 : 0) : (playing[1] ? 1 : 0);
    const auto* record = library != nullptr && playing[deck] ? library->find(loadedId[(size_t) deck]) : nullptr;
    const auto text = formatBpm(record != nullptr ? record->bpm : std::nullopt, false);

    if (text != masterBpm)
    {
        masterBpm = text;
        view.topBar().setMasterBpm(text);
    }
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

        // Repaint only when the 50 ms bar under the playhead changes.
        const auto d = static_cast<std::size_t>(deck);
        const auto position = snapshot.positionSeconds.load();
        const auto bar = static_cast<int>(std::lround(position * 20.0));
        if (! peaks[d].empty() && bar != shownBar[d])
        {
            shownBar[d] = bar;
            view.waveforms().waveform(deck).setBars(barsAround(peaks[d], position));
        }
    }

    updateMasterBpm();

    engine.collectRetired();
}
