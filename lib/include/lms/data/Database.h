#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace lms {

/// RAII wrapper around a prepared statement. All values reach SQLite through
/// bind() calls - SQL text is never built by concatenating user input (NFR-06).
class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&&) = delete;
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement& bind(int index, int value);
    Statement& bind(int index, std::int64_t value);
    Statement& bind(int index, double value);
    Statement& bind(int index, bool value);
    Statement& bind(int index, const std::string& value);
    Statement& bind(int index, const char* value);
    Statement& bindNull(int index);

    template <typename T>
    Statement& bind(int index, const std::optional<T>& value) {
        return value ? bind(index, *value) : bindNull(index);
    }

    /// Executes one step. Returns true when a row is available.
    bool step();
    /// Executes a statement that returns no rows (INSERT/UPDATE/DELETE).
    void run();

    bool isNull(int column) const;
    int columnInt(int column) const;
    std::int64_t columnInt64(int column) const;
    double columnDouble(int column) const;
    bool columnBool(int column) const { return columnInt(column) != 0; }
    std::string columnText(int column) const;
    std::optional<int> columnOptionalInt(int column) const;
    std::optional<std::int64_t> columnOptionalInt64(int column) const;
    std::optional<double> columnOptionalDouble(int column) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_;
};

/// Owns one SQLite connection. Enables foreign keys, WAL journaling and a busy timeout.
class Database {
public:
    /// Opens (creating if necessary) the database file; ":memory:" gives a private in-memory DB.
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void execute(const std::string& sql);
    Statement prepare(const std::string& sql);
    std::int64_t lastInsertId() const;
    int changes() const;

    int userVersion();
    void setUserVersion(int version);

    /// Runs `work` inside a SAVEPOINT; commits on success, rolls back and rethrows on
    /// exception. Savepoints nest, so services can call each other safely.
    void inTransaction(const std::function<void()>& work);

private:
    sqlite3* db_ = nullptr;
    int savepointDepth_ = 0;
};

/// Unit-of-work abstraction used by the service layer, so services do not depend on SQLite.
class ITransactionRunner {
public:
    virtual ~ITransactionRunner() = default;
    virtual void run(const std::function<void()>& work) = 0;
};

class SqliteTransactionRunner final : public ITransactionRunner {
public:
    explicit SqliteTransactionRunner(Database& db) : db_(db) {}
    void run(const std::function<void()>& work) override { db_.inTransaction(work); }

private:
    Database& db_;
};

/// Applies the numbered migrations in sql/ (embedded at build time) using PRAGMA user_version.
class SchemaMigrator {
public:
    static int latestVersion();
    /// Brings the database up to latestVersion(). Throws DatabaseError if the file was
    /// created by a newer release than this program.
    static void migrate(Database& db);
};

}  // namespace lms
