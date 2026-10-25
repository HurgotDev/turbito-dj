#include "library/db/Schema.h"

#include <array>

namespace
{
    // Append only: index + 1 is the user_version a script leaves behind.
    constexpr std::array migrations {
        R"sql(
            CREATE TABLE folders(
                id INTEGER PRIMARY KEY,
                path TEXT NOT NULL UNIQUE);
            CREATE TABLE tracks(
                id INTEGER PRIMARY KEY,
                path TEXT NOT NULL UNIQUE,
                folder_id INTEGER REFERENCES folders(id),
                title TEXT NOT NULL,
                artist TEXT NOT NULL DEFAULT '',
                duration REAL NOT NULL DEFAULT 0,
                bpm REAL,
                first_beat REAL,
                camelot TEXT,
                waveform BLOB,
                analyzed INTEGER NOT NULL DEFAULT 0);
            CREATE TABLE history(
                id INTEGER PRIMARY KEY,
                track_id INTEGER NOT NULL REFERENCES tracks(id),
                loaded_at INTEGER NOT NULL);
        )sql",
    };
}

void migrate(Database& db)
{
    Database::Statement version(db, "PRAGMA user_version");
    version.step();
    const auto current = static_cast<size_t>(version.getInt(0));

    for (auto i = current; i < migrations.size(); ++i)
    {
        Database::Transaction transaction(db);
        db.exec(migrations[i]);
        db.exec("PRAGMA user_version = " + juce::String(static_cast<int>(i + 1)));
        transaction.commit();
    }
}
