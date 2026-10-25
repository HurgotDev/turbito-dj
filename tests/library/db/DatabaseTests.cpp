#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "library/db/Database.h"

#include <stdexcept>

using Catch::Matchers::WithinAbs;

namespace
{
    struct TempDb
    {
        juce::File file = juce::File::createTempFile(".db");
        ~TempDb()
        {
            file.deleteFile();
            file.withFileExtension(".db-wal").deleteFile();
            file.withFileExtension(".db-shm").deleteFile();
        }
    };

    juce::int64 countRows(Database& db)
    {
        Database::Statement count(db, "SELECT COUNT(*) FROM t");
        REQUIRE(count.step());
        return count.getInt64(0);
    }
}

TEST_CASE("Database opens a file in WAL mode with foreign keys on", "[library][db]")
{
    TempDb temp;
    Database db;
    db.open(temp.file);

    Database::Statement journal(db, "PRAGMA journal_mode");
    REQUIRE(journal.step());
    CHECK(journal.getText(0) == "wal");

    Database::Statement keys(db, "PRAGMA foreign_keys");
    REQUIRE(keys.step());
    CHECK(keys.getInt(0) == 1);
}

TEST_CASE("Database creates the parent folder", "[library][db]")
{
    auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                   .getNonexistentChildFile("turbito-db", "");
    {
        Database db;
        db.open(dir.getChildFile("library.db"));
    }
    CHECK(dir.getChildFile("library.db").existsAsFile());
    dir.deleteRecursively();
}

TEST_CASE("Statement binds and reads every column type", "[library][db]")
{
    TempDb temp;
    Database db;
    db.open(temp.file);
    db.exec("CREATE TABLE v(i INTEGER, l INTEGER, d REAL, s TEXT, b BLOB, n TEXT)");

    const std::vector<std::uint8_t> bytes { 0, 1, 127, 255 };
    {
        Database::Statement insert(db, "INSERT INTO v VALUES (?, ?, ?, ?, ?, ?)");
        insert.bind(1, 42);
        insert.bind(2, juce::int64(1) << 40);
        insert.bind(3, 124.5);
        insert.bind(4, juce::String(juce::CharPointer_UTF8("Canci\xc3\xb3n")));
        insert.bind(5, bytes);
        insert.bindNull(6);
        CHECK_FALSE(insert.step());
    }

    Database::Statement select(db, "SELECT i, l, d, s, b, n FROM v");
    REQUIRE(select.step());
    CHECK(select.getInt(0) == 42);
    CHECK(select.getInt64(1) == (juce::int64(1) << 40));
    CHECK_THAT(select.getDouble(2), WithinAbs(124.5, 1e-9));
    CHECK(select.getText(3) == juce::String(juce::CharPointer_UTF8("Canci\xc3\xb3n")));
    CHECK(select.getBlob(4) == bytes);
    CHECK(select.isNull(5));
    CHECK_FALSE(select.isNull(0));
    CHECK_FALSE(select.step());

    select.reset();
    CHECK(select.step());
}

TEST_CASE("Database reports SQL errors as exceptions", "[library][db]")
{
    TempDb temp;
    Database db;
    db.open(temp.file);
    CHECK_THROWS_AS(db.exec("NOT SQL"), std::runtime_error);
    CHECK_THROWS_AS(Database::Statement(db, "SELECT * FROM missing"), std::runtime_error);
}

TEST_CASE("Transaction commits or rolls back on exception", "[library][db]")
{
    TempDb temp;
    Database db;
    db.open(temp.file);
    db.exec("CREATE TABLE t(x INTEGER)");

    {
        Database::Transaction transaction(db);
        db.exec("INSERT INTO t VALUES (1)");
        transaction.commit();
    }
    CHECK(countRows(db) == 1);

    try
    {
        Database::Transaction transaction(db);
        db.exec("INSERT INTO t VALUES (2)");
        throw std::runtime_error("import failed");
    }
    catch (const std::runtime_error&) {}
    CHECK(countRows(db) == 1);
}
