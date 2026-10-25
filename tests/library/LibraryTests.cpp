#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "library/Library.h"

#include "MessageLoop.h"

using Catch::Matchers::WithinAbs;

namespace
{
    struct TempFiles
    {
        juce::File db = juce::File::createTempFile(".db");
        juce::File music = juce::File::getSpecialLocation(juce::File::tempDirectory)
                               .getNonexistentChildFile("turbito-library", "");

        TempFiles()
        {
            const juce::File source(TURBITO_SOURCE_DIR "/test-audio");
            music.getChildFile("sub").createDirectory();
            source.getChildFile("beat_124bpm_30s.wav").copyFileTo(music.getChildFile("beat_124bpm_30s.wav"));
            source.getChildFile("beat_126bpm_30s.wav").copyFileTo(music.getChildFile("sub/beat_126bpm_30s.wav"));
            music.getChildFile("notes.txt").replaceWithText("not audio");
        }

        ~TempFiles()
        {
            music.deleteRecursively();
            db.deleteFile();
            db.withFileExtension(".db-wal").deleteFile();
            db.withFileExtension(".db-shm").deleteFile();
        }
    };

    bool allAnalyzed(const Library& library)
    {
        const auto& r = library.records();
        return r.size() == 2 && std::all_of(r.begin(), r.end(), [](const auto& t) { return t.analyzed && t.bpm; });
    }
}

TEST_CASE("Library imports a folder, analyses it and keeps it after reopening", "[library]")
{
    const juce::ScopedJuceInitialiser_GUI juce;
    TempFiles files;
    const auto first = files.music.getChildFile("beat_124bpm_30s.wav");

    {
        Library library(files.db);
        int changes = 0;
        library.onChanged = [&] { ++changes; };

        library.importFolder(files.music);
        runMessageLoopUntil([&] { return allAnalyzed(library); }, 60000);

        REQUIRE(allAnalyzed(library));
        CHECK(library.folderCount() == 1);
        CHECK(changes >= 1);
        REQUIRE(library.find(first) != nullptr);
        CHECK_THAT(*library.find(first)->bpm, WithinAbs(124.0, 0.1));
        CHECK_FALSE(library.waveform(library.find(first)->id).empty());

        // Reimporting finds nothing new but still reports it is done.
        const auto before = changes;
        library.importFolder(files.music);
        runMessageLoopUntil([&] { return changes > before; }, 10000);
        CHECK(changes > before);
        CHECK(library.records().size() == 2);
        CHECK(library.folderCount() == 1);

        library.addHistory(library.find(first)->id);
    }

    Library reopened(files.db);
    CHECK(allAnalyzed(reopened));
    CHECK(reopened.folderCount() == 1);
    REQUIRE(reopened.find(first) != nullptr);
    CHECK_THAT(*reopened.find(first)->bpm, WithinAbs(124.0, 0.1));
    CHECK(reopened.history() == std::vector<juce::int64> { reopened.find(first)->id });
}
