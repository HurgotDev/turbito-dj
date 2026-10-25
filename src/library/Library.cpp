#include "library/Library.h"

#include "library/db/Schema.h"
#include "library/scan/FolderScanner.h"
#include "library/scan/TagReader.h"

#include <algorithm>
#include <set>

Library::Library(const juce::File& databaseFile)
{
    db.open(databaseFile);
    migrate(db);
    tracks = repo.allTracks();
    folders = static_cast<int>(repo.folders().size());
    analysis.onResult = [this](juce::int64 id, std::optional<TrackAnalysis> result) { handleAnalysis(id, std::move(result)); };
}

Library::~Library()
{
    *alive = false;
    cancelPendingUpdate();
}

const LibraryRecord* Library::find(juce::int64 id) const
{
    const auto it = std::find_if(tracks.begin(), tracks.end(), [id](const auto& r) { return r.id == id; });
    return it != tracks.end() ? &*it : nullptr;
}

const LibraryRecord* Library::find(const juce::File& file) const
{
    const auto it = std::find_if(tracks.begin(), tracks.end(), [&file](const auto& r) { return r.file == file; });
    return it != tracks.end() ? &*it : nullptr;
}

std::vector<juce::int64> Library::history()
{
    return repo.history();
}

void Library::importFolder(const juce::File& folder)
{
    std::set<juce::String> paths;
    for (const auto& r : tracks)
        paths.insert(r.file.getFullPathName());

    juce::Thread::launch([this, folder, known = std::move(paths), weakAlive = std::weak_ptr<bool>(alive)]
    {
        std::vector<Scanned> scanned;

        for (const auto& file : findAudioFiles(folder))
            if (known.count(file.getFullPathName()) == 0)
                if (const auto tags = readTags(file))
                    scanned.push_back({ file, tags->title, tags->artist, tags->duration });

        juce::MessageManager::callAsync([this, folder, found = std::move(scanned), weakAlive]
        {
            // alive is only reset on the message thread, so the check holds for this call.
            if (const auto flag = weakAlive.lock(); flag != nullptr && *flag)
                addScanned(folder, found);
        });
    });
}

void Library::addScanned(const juce::File& folder, const std::vector<Scanned>& files)
{
    const auto firstNew = tracks.size();
    {
        Database::Transaction transaction(db);
        const auto folderId = repo.addFolder(folder);

        for (const auto& f : files)
        {
            if (const auto id = repo.addTrack(folderId, f.file, f.title, f.artist, f.duration))
            {
                LibraryRecord record;
                record.id = *id;
                record.file = f.file;
                record.title = f.title;
                record.artist = f.artist;
                record.duration = f.duration;
                record.folderId = folderId;
                tracks.push_back(std::move(record));
            }
        }

        transaction.commit();
    }

    folders = static_cast<int>(repo.folders().size());

    for (auto i = firstNew; i < tracks.size(); ++i)
        analysis.enqueue(tracks[i].id, tracks[i].file);

    triggerAsyncUpdate();
}

void Library::analyzePending()
{
    for (const auto& r : tracks)
        if (! r.analyzed)
            analysis.enqueue(r.id, r.file);
}

void Library::handleAnalysis(juce::int64 id, std::optional<TrackAnalysis> result)
{
    const auto it = std::find_if(tracks.begin(), tracks.end(), [id](const auto& r) { return r.id == id; });
    if (! result || it == tracks.end())
        return;

    repo.saveAnalysis(id, *result);
    // Same rule as saveAnalysis: bpm 0 is unknown.
    it->bpm = result->bpm > 0.0 ? std::optional(result->bpm) : std::nullopt;
    it->firstBeat = result->bpm > 0.0 ? std::optional(result->firstBeat) : std::nullopt;
    it->camelot = result->camelot;
    it->analyzed = true;
    triggerAsyncUpdate();
}

std::vector<std::uint8_t> Library::waveform(juce::int64 id)
{
    return repo.waveform(id);
}

void Library::addHistory(juce::int64 id)
{
    repo.addHistory(id, juce::Time::currentTimeMillis());
}

void Library::handleAsyncUpdate()
{
    if (onChanged)
        onChanged();
}
