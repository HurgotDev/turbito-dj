#pragma once

#include "library/Track.h"
#include "library/analysis/TrackAnalysis.h"
#include "library/db/Database.h"

#include <optional>
#include <vector>

// Reads and writes the library tables. Wrap bulk inserts in a Database::Transaction.
class TrackRepository
{
public:
    explicit TrackRepository(Database& database) : db(database) {}

    // Inserts the folder or returns the id it already has.
    juce::int64 addFolder(const juce::File& folder);
    std::vector<juce::File> folders();

    // nullopt when the path is already in the library.
    std::optional<juce::int64> addTrack(juce::int64 folderId, const juce::File& file,
                                        const juce::String& title, const juce::String& artist,
                                        double duration);
    std::vector<LibraryRecord> allTracks();

    // Marks the track analysed; bpm 0 is stored as unknown.
    void saveAnalysis(juce::int64 trackId, const TrackAnalysis& analysis);
    std::vector<std::uint8_t> waveform(juce::int64 trackId);

    void addHistory(juce::int64 trackId, juce::int64 timeMs);
    // Distinct track ids, most recently loaded first.
    std::vector<juce::int64> history();

private:
    Database& db;
};
