#include <catch2/catch_test_macros.hpp>

#include <sqlite3.h>
#include <taglib/fileref.h>

#include <dsp/keydetection/GetKeyMode.h>
#include <dsp/tempotracking/TempoTrackV2.h>

TEST_CASE("SQLite amalgamation opens an in-memory database", "[library][deps]")
{
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(":memory:", &db) == SQLITE_OK);

    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, "select 1", -1, &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    CHECK(sqlite3_column_int(stmt, 0) == 1);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

TEST_CASE("TagLib reports a missing file as null", "[library][deps]")
{
    const TagLib::FileRef file("turbito-missing-file-does-not-exist.mp3");
    CHECK(file.isNull());
}

TEST_CASE("qm-dsp key and tempo analysers construct", "[library][deps]")
{
    CHECK_NOTHROW(GetKeyMode(GetKeyMode::Config(44100.0, 440.0f)));
    CHECK_NOTHROW(TempoTrackV2(44100.0f, 512));
}
