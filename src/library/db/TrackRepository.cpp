#include "library/db/TrackRepository.h"

juce::int64 TrackRepository::addFolder(const juce::File& folder)
{
    const auto path = folder.getFullPathName();

    Database::Statement insert(db, "INSERT OR IGNORE INTO folders(path) VALUES (?)");
    insert.bind(1, path);
    insert.step();

    Database::Statement select(db, "SELECT id FROM folders WHERE path = ?");
    select.bind(1, path);
    select.step();
    return select.getInt64(0);
}

std::vector<juce::File> TrackRepository::folders()
{
    std::vector<juce::File> result;
    Database::Statement select(db, "SELECT path FROM folders ORDER BY id");
    while (select.step())
        result.emplace_back(select.getText(0));
    return result;
}

std::optional<juce::int64> TrackRepository::addTrack(juce::int64 folderId, const juce::File& file,
                                                     const juce::String& title, const juce::String& artist,
                                                     double duration)
{
    Database::Statement insert(db, "INSERT OR IGNORE INTO tracks(folder_id, path, title, artist, duration) "
                                   "VALUES (?, ?, ?, ?, ?)");
    insert.bind(1, folderId);
    insert.bind(2, file.getFullPathName());
    insert.bind(3, title);
    insert.bind(4, artist);
    insert.bind(5, duration);
    insert.step();

    if (db.changes() == 0)
        return std::nullopt;
    return db.lastInsertId();
}

std::vector<LibraryRecord> TrackRepository::allTracks()
{
    std::vector<LibraryRecord> result;
    Database::Statement select(db, "SELECT id, path, title, artist, duration, bpm, first_beat, camelot, "
                                   "analyzed, folder_id FROM tracks ORDER BY id");
    while (select.step())
    {
        LibraryRecord& r = result.emplace_back();
        r.id = select.getInt64(0);
        r.file = juce::File(select.getText(1));
        r.title = select.getText(2);
        r.artist = select.getText(3);
        r.duration = select.getDouble(4);
        if (! select.isNull(5))
            r.bpm = select.getDouble(5);
        if (! select.isNull(6))
            r.firstBeat = select.getDouble(6);
        r.camelot = select.getText(7);
        r.analyzed = select.getInt(8) != 0;
        r.folderId = select.getInt64(9);
    }
    return result;
}

void TrackRepository::saveAnalysis(juce::int64 trackId, const TrackAnalysis& analysis)
{
    Database::Statement update(db, "UPDATE tracks SET bpm = ?, first_beat = ?, camelot = ?, waveform = ?, "
                                   "analyzed = 1 WHERE id = ?");
    // No tempo means no beatgrid either.
    if (analysis.bpm > 0.0)
    {
        update.bind(1, analysis.bpm);
        update.bind(2, analysis.firstBeat);
    }
    else
    {
        update.bindNull(1);
        update.bindNull(2);
    }
    if (analysis.camelot.isEmpty())
        update.bindNull(3);
    else
        update.bind(3, analysis.camelot);
    update.bind(4, analysis.waveform);
    update.bind(5, trackId);
    update.step();
}

std::vector<std::uint8_t> TrackRepository::waveform(juce::int64 trackId)
{
    Database::Statement select(db, "SELECT waveform FROM tracks WHERE id = ?");
    select.bind(1, trackId);
    return select.step() ? select.getBlob(0) : std::vector<std::uint8_t> {};
}

void TrackRepository::addHistory(juce::int64 trackId, juce::int64 timeMs)
{
    Database::Statement insert(db, "INSERT INTO history(track_id, loaded_at) VALUES (?, ?)");
    insert.bind(1, trackId);
    insert.bind(2, timeMs);
    insert.step();
}

std::vector<juce::int64> TrackRepository::history()
{
    std::vector<juce::int64> result;
    Database::Statement select(db, "SELECT track_id FROM history GROUP BY track_id "
                                   "ORDER BY MAX(loaded_at) DESC, MAX(id) DESC");
    while (select.step())
        result.push_back(select.getInt64(0));
    return result;
}
