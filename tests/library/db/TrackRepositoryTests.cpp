#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "library/db/Database.h"
#include "library/db/Schema.h"
#include "library/db/TrackRepository.h"

using Catch::Matchers::WithinAbs;

namespace
{
    struct TempLibrary
    {
        juce::File file = juce::File::createTempFile(".db");
        Database db;
        TrackRepository repo { db };

        TempLibrary()
        {
            db.open(file);
            migrate(db);
        }

        ~TempLibrary()
        {
            db.close();
            file.deleteFile();
            file.withFileExtension(".db-wal").deleteFile();
            file.withFileExtension(".db-shm").deleteFile();
        }
    };

    const juce::File musicDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("turbito-music");
}

TEST_CASE("addFolder returns the same id for the same path", "[library][db]")
{
    TempLibrary lib;
    const auto id = lib.repo.addFolder(musicDir);
    CHECK(lib.repo.addFolder(musicDir) == id);
    CHECK(lib.repo.addFolder(musicDir.getChildFile("otra")) != id);
    CHECK(lib.repo.folders() == std::vector<juce::File> { musicDir, musicDir.getChildFile("otra") });
}

TEST_CASE("addTrack inserts once per path and allTracks reads it back", "[library][db]")
{
    TempLibrary lib;
    const auto folder = lib.repo.addFolder(musicDir);
    const auto file = musicDir.getChildFile("la_pollera.mp3");

    const auto id = lib.repo.addTrack(folder, file, "La pollera colora", "Wilson Choperena", 182.5);
    REQUIRE(id.has_value());
    CHECK_FALSE(lib.repo.addTrack(folder, file, "Otro", "Otro", 1.0).has_value());

    const auto tracks = lib.repo.allTracks();
    REQUIRE(tracks.size() == 1);
    const auto& t = tracks[0];
    CHECK(t.id == *id);
    CHECK(t.file == file);
    CHECK(t.title == "La pollera colora");
    CHECK(t.artist == "Wilson Choperena");
    CHECK_THAT(t.duration, WithinAbs(182.5, 1e-9));
    CHECK_FALSE(t.bpm.has_value());
    CHECK_FALSE(t.firstBeat.has_value());
    CHECK(t.camelot.isEmpty());
    CHECK_FALSE(t.analyzed);
    CHECK(t.folderId == folder);
}

TEST_CASE("saveAnalysis stores results and the waveform bytes", "[library][db]")
{
    TempLibrary lib;
    const auto folder = lib.repo.addFolder(musicDir);
    const auto id = *lib.repo.addTrack(folder, musicDir.getChildFile("a.wav"), "A", "", 30.0);

    std::vector<std::uint8_t> peaks(600);
    for (size_t i = 0; i < peaks.size(); ++i)
        peaks[i] = static_cast<std::uint8_t>(i % 256);

    lib.repo.saveAnalysis(id, { 124.0, 0.012, "8A", peaks });

    const auto t = lib.repo.allTracks().at(0);
    REQUIRE(t.bpm.has_value());
    CHECK_THAT(*t.bpm, WithinAbs(124.0, 1e-9));
    REQUIRE(t.firstBeat.has_value());
    CHECK_THAT(*t.firstBeat, WithinAbs(0.012, 1e-9));
    CHECK(t.camelot == "8A");
    CHECK(t.analyzed);
    CHECK(lib.repo.waveform(id) == peaks);
}

TEST_CASE("saveAnalysis with undetected BPM leaves bpm empty", "[library][db]")
{
    TempLibrary lib;
    const auto folder = lib.repo.addFolder(musicDir);
    const auto id = *lib.repo.addTrack(folder, musicDir.getChildFile("ruido.wav"), "Ruido", "", 5.0);

    lib.repo.saveAnalysis(id, { 0.0, 0.0, "", {} });

    const auto t = lib.repo.allTracks().at(0);
    CHECK(t.analyzed);
    CHECK_FALSE(t.bpm.has_value());
    CHECK_FALSE(t.firstBeat.has_value());
    CHECK(lib.repo.waveform(id).empty());
}

TEST_CASE("Data persists after reopening the file", "[library][db]")
{
    auto file = juce::File::createTempFile(".db");
    juce::int64 id = 0;
    {
        Database db;
        db.open(file);
        migrate(db);
        TrackRepository repo(db);
        Database::Transaction transaction(db);
        const auto folder = repo.addFolder(musicDir);
        for (int i = 0; i < 1000; ++i)
            repo.addTrack(folder, musicDir.getChildFile(juce::String(i) + ".mp3"), juce::String(i), "", 1.0);
        id = *repo.addTrack(folder, musicDir.getChildFile("ultima.mp3"), "Ultima", "", 2.0);
        transaction.commit();
        repo.saveAnalysis(id, { 126.0, 0.25, "9B", { 9, 8, 7 } });
    }
    {
        Database db;
        db.open(file);
        migrate(db);
        TrackRepository repo(db);
        CHECK(repo.allTracks().size() == 1001);
        CHECK(repo.waveform(id) == std::vector<std::uint8_t> { 9, 8, 7 });
        CHECK(repo.folders() == std::vector<juce::File> { musicDir });
    }
    file.deleteFile();
    file.withFileExtension(".db-wal").deleteFile();
    file.withFileExtension(".db-shm").deleteFile();
}

TEST_CASE("history lists distinct tracks, most recent first", "[library][db]")
{
    TempLibrary lib;
    const auto folder = lib.repo.addFolder(musicDir);
    const auto a = *lib.repo.addTrack(folder, musicDir.getChildFile("a.mp3"), "A", "", 1.0);
    const auto b = *lib.repo.addTrack(folder, musicDir.getChildFile("b.mp3"), "B", "", 1.0);
    const auto c = *lib.repo.addTrack(folder, musicDir.getChildFile("c.mp3"), "C", "", 1.0);

    lib.repo.addHistory(a, 1000);
    lib.repo.addHistory(b, 2000);
    lib.repo.addHistory(c, 3000);
    lib.repo.addHistory(a, 4000);

    CHECK(lib.repo.history() == std::vector<juce::int64> { a, c, b });
}
