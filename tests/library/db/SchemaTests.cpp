#include <catch2/catch_test_macros.hpp>

#include "library/db/Database.h"
#include "library/db/Schema.h"

namespace
{
    int userVersion(Database& db)
    {
        Database::Statement version(db, "PRAGMA user_version");
        REQUIRE(version.step());
        return version.getInt(0);
    }
}

TEST_CASE("migrate brings a new database to version 1 and is idempotent", "[library][db]")
{
    auto file = juce::File::createTempFile(".db");
    {
        Database db;
        db.open(file);
        REQUIRE(userVersion(db) == 0);

        migrate(db);
        CHECK(userVersion(db) == 1);
        CHECK_NOTHROW(db.exec("SELECT id, path FROM folders"));
        CHECK_NOTHROW(db.exec("SELECT id, path, folder_id, title, artist, duration, bpm, "
                              "first_beat, camelot, waveform, analyzed FROM tracks"));
        CHECK_NOTHROW(db.exec("SELECT id, track_id, loaded_at FROM history"));

        CHECK_NOTHROW(migrate(db));
        CHECK(userVersion(db) == 1);
    }
    file.deleteFile();
    file.withFileExtension(".db-wal").deleteFile();
    file.withFileExtension(".db-shm").deleteFile();
}
