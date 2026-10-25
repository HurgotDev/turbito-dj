#pragma once

#include <juce_core/juce_core.h>

#include <cstdint>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

// One SQLite connection. Message thread only; errors throw std::runtime_error.
class Database
{
public:
    Database() = default;
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Creates the parent folder. WAL journal and foreign keys on.
    void open(const juce::File& file);
    void close();

    void exec(const juce::String& sql);
    juce::int64 lastInsertId() const;
    int changes() const;

    class Statement
    {
    public:
        Statement(Database& db, const char* sql);
        ~Statement();

        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;

        // Indices are 1-based, as in SQLite.
        void bind(int index, int value);
        void bind(int index, juce::int64 value);
        void bind(int index, double value);
        void bind(int index, const juce::String& value);
        void bind(int index, const std::vector<std::uint8_t>& value);
        void bindNull(int index);

        // True while there is a row to read; false when done.
        bool step();
        void reset();

        // Column indices are 0-based, as in SQLite.
        bool isNull(int column) const;
        int getInt(int column) const;
        juce::int64 getInt64(int column) const;
        double getDouble(int column) const;
        juce::String getText(int column) const;
        std::vector<std::uint8_t> getBlob(int column) const;

    private:
        void check(int result) const;

        sqlite3* db;
        sqlite3_stmt* stmt = nullptr;
    };

    // Rolls back unless commit() was called, e.g. when an exception unwinds.
    class Transaction
    {
    public:
        explicit Transaction(Database& db);
        ~Transaction();

        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;

        void commit();

    private:
        Database& db;
        bool done = false;
    };

private:
    sqlite3* db = nullptr;
};
