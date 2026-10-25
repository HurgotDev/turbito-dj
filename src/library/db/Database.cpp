#include "library/db/Database.h"

#include <sqlite3.h>

#include <stdexcept>

namespace
{
    [[noreturn]] void fail(sqlite3* db)
    {
        throw std::runtime_error(db != nullptr ? sqlite3_errmsg(db) : "SQLite: out of memory");
    }
}

Database::~Database()
{
    close();
}

void Database::open(const juce::File& file)
{
    close();
    file.getParentDirectory().createDirectory();

    if (sqlite3_open(file.getFullPathName().toRawUTF8(), &db) != SQLITE_OK)
    {
        const std::runtime_error error(db != nullptr ? sqlite3_errmsg(db) : "SQLite: out of memory");
        close();
        throw error;
    }

    exec("PRAGMA journal_mode=WAL; PRAGMA foreign_keys=ON;");
}

void Database::close()
{
    sqlite3_close(db);
    db = nullptr;
}

void Database::exec(const juce::String& sql)
{
    if (sqlite3_exec(db, sql.toRawUTF8(), nullptr, nullptr, nullptr) != SQLITE_OK)
        fail(db);
}

juce::int64 Database::lastInsertId() const
{
    return sqlite3_last_insert_rowid(db);
}

int Database::changes() const
{
    return sqlite3_changes(db);
}

Database::Statement::Statement(Database& owner, const char* sql)
    : db(owner.db)
{
    check(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr));
}

Database::Statement::~Statement()
{
    sqlite3_finalize(stmt);
}

void Database::Statement::check(int result) const
{
    if (result != SQLITE_OK)
        fail(db);
}

void Database::Statement::bind(int index, int value)
{
    check(sqlite3_bind_int(stmt, index, value));
}

void Database::Statement::bind(int index, juce::int64 value)
{
    check(sqlite3_bind_int64(stmt, index, value));
}

void Database::Statement::bind(int index, double value)
{
    check(sqlite3_bind_double(stmt, index, value));
}

void Database::Statement::bind(int index, const juce::String& value)
{
    check(sqlite3_bind_text(stmt, index, value.toRawUTF8(), -1, SQLITE_TRANSIENT));
}

void Database::Statement::bind(int index, const std::vector<std::uint8_t>& value)
{
    // zeroblob: an empty vector stays an empty blob instead of NULL.
    if (value.empty())
        check(sqlite3_bind_zeroblob(stmt, index, 0));
    else
        check(sqlite3_bind_blob(stmt, index, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT));
}

void Database::Statement::bindNull(int index)
{
    check(sqlite3_bind_null(stmt, index));
}

bool Database::Statement::step()
{
    const auto result = sqlite3_step(stmt);
    if (result == SQLITE_ROW)
        return true;
    if (result == SQLITE_DONE)
        return false;
    fail(db);
}

void Database::Statement::reset()
{
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
}

bool Database::Statement::isNull(int column) const
{
    return sqlite3_column_type(stmt, column) == SQLITE_NULL;
}

int Database::Statement::getInt(int column) const
{
    return sqlite3_column_int(stmt, column);
}

juce::int64 Database::Statement::getInt64(int column) const
{
    return sqlite3_column_int64(stmt, column);
}

double Database::Statement::getDouble(int column) const
{
    return sqlite3_column_double(stmt, column);
}

juce::String Database::Statement::getText(int column) const
{
    return juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, column)),
                                  sqlite3_column_bytes(stmt, column));
}

std::vector<std::uint8_t> Database::Statement::getBlob(int column) const
{
    const auto* data = static_cast<const std::uint8_t*>(sqlite3_column_blob(stmt, column));
    return data != nullptr ? std::vector<std::uint8_t>(data, data + sqlite3_column_bytes(stmt, column))
                           : std::vector<std::uint8_t> {};
}

Database::Transaction::Transaction(Database& owner)
    : db(owner)
{
    db.exec("BEGIN");
}

Database::Transaction::~Transaction()
{
    // Not exec(): a destructor must not throw.
    if (! done)
        sqlite3_exec(db.db, "ROLLBACK", nullptr, nullptr, nullptr);
}

void Database::Transaction::commit()
{
    db.exec("COMMIT");
    done = true;
}
