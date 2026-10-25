#pragma once

#include "library/Track.h"
#include "library/analysis/AnalysisQueue.h"
#include "library/db/Database.h"
#include "library/db/TrackRepository.h"

#include <juce_events/juce_events.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

// The library on the message thread: the tracks in memory, SQLite as their
// store, folder imports on a background thread and the analysis queue.
class Library : private juce::AsyncUpdater
{
public:
    // Opens (or creates) the database and loads every track. Throws std::runtime_error.
    explicit Library(const juce::File& databaseFile);
    ~Library() override;

    const std::vector<LibraryRecord>& records() const noexcept { return tracks; }
    const LibraryRecord* find(juce::int64 id) const;
    const LibraryRecord* find(const juce::File& file) const;
    int folderCount() const noexcept { return folders; }
    // Track ids, most recently loaded first.
    std::vector<juce::int64> history();

    // Scans the folder (subfolders too) and reads tags on a background thread,
    // then adds the new tracks in one transaction and queues them for analysis.
    void importFolder(const juce::File& folder);
    // Queues every track not analysed yet.
    void analyzePending();

    std::vector<std::uint8_t> waveform(juce::int64 id);
    void addHistory(juce::int64 id);

    // After an import or analysis results; coalesced, on the message thread.
    std::function<void()> onChanged;

private:
    struct Scanned
    {
        juce::File file;
        juce::String title, artist;
        double duration;
    };

    void addScanned(const juce::File& folder, const std::vector<Scanned>& files);
    void handleAnalysis(juce::int64 id, std::optional<TrackAnalysis> analysis);
    void handleAsyncUpdate() override;

    Database db;
    TrackRepository repo { db };
    std::vector<LibraryRecord> tracks;
    int folders = 0;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
    // Last: destroyed first, so no result arrives once the rest is gone.
    AnalysisQueue analysis;
};
