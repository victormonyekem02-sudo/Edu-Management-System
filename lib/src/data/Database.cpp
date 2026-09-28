#include "lms/data/Database.h"

#include <sqlite3.h>

#include "lms/common/Errors.h"
#include "lms/data/GeneratedMigrations.h"

namespace lms {

namespace {

[[noreturn]] void throwDatabaseError(sqlite3* db, const std::string& context) {
    const std::string detail = db != nullptr ? sqlite3_errmsg(db) : "unknown error";
    const int code = db != nullptr ? sqlite3_extended_errcode(db) : SQLITE_ERROR;
    if ((code & 0xff) == SQLITE_CONSTRAINT) {
        throw DatabaseError(context + ": constraint violated (" + detail + ")");
    }
    throw DatabaseError(context + ": " + detail);
}

}  // namespace

// ---- Statement ---------------------------------------------------------------

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db), stmt_(nullptr) {
    if (sqlite3_prepare_v2(db_, sql.c_str(), static_cast<int>(sql.size()), &stmt_, nullptr) !=
        SQLITE_OK) {
        throwDatabaseError(db_, "Failed to prepare statement");
    }
}

Statement::~Statement() {
    if (stmt_ != nullptr) {
        sqlite3_finalize(stmt_);
    }
}

Statement::Statement(Statement&& other) noexcept : db_(other.db_), stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

Statement& Statement::bind(int index, int value) {
    if (sqlite3_bind_int(stmt_, index, value) != SQLITE_OK) {
        throwDatabaseError(db_, "Failed to bind parameter");
    }
    return *this;
}

Statement& Statement::bind(int index, std::int64_t value) {
    if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK) {
        throwDatabaseError(db_, "Failed to bind parameter");
    }
    return *this;
}

Statement& Statement::bind(int index, double value) {
    if (sqlite3_bind_double(stmt_, index, value) != SQLITE_OK) {
        throwDatabaseError(db_, "Failed to bind parameter");
    }
    return *this;
}

Statement& Statement::bind(int index, bool value) {
    return bind(index, value ? 1 : 0);
}

Statement& Statement::bind(int index, const std::string& value) {
    if (sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()),
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        throwDatabaseError(db_, "Failed to bind parameter");
    }
    return *this;
}

Statement& Statement::bind(int index, const char* value) {
    return bind(index, std::string(value));
}

Statement& Statement::bindNull(int index) {
    if (sqlite3_bind_null(stmt_, index) != SQLITE_OK) {
        throwDatabaseError(db_, "Failed to bind parameter");
    }
    return *this;
}

bool Statement::step() {
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) {
        return true;
    }
    if (rc == SQLITE_DONE) {
        return false;
    }
    throwDatabaseError(db_, "Database operation failed");
}

void Statement::run() {
    while (step()) {
    }
}

bool Statement::isNull(int column) const {
    return sqlite3_column_type(stmt_, column) == SQLITE_NULL;
}

int Statement::columnInt(int column) const {
    return sqlite3_column_int(stmt_, column);
}

std::int64_t Statement::columnInt64(int column) const {
    return sqlite3_column_int64(stmt_, column);
}

double Statement::columnDouble(int column) const {
    return sqlite3_column_double(stmt_, column);
}

std::string Statement::columnText(int column) const {
    const unsigned char* text = sqlite3_column_text(stmt_, column);
    if (text == nullptr) {
        return {};
    }
    // SQLite's C API returns UTF-8 text as unsigned char*; this is the documented conversion.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const char*>(text),
            static_cast<std::size_t>(sqlite3_column_bytes(stmt_, column))};
}

std::optional<int> Statement::columnOptionalInt(int column) const {
    if (isNull(column)) {
        return std::nullopt;
    }
    return columnInt(column);
}

std::optional<std::int64_t> Statement::columnOptionalInt64(int column) const {
    if (isNull(column)) {
        return std::nullopt;
    }
    return columnInt64(column);
}

std::optional<double> Statement::columnOptionalDouble(int column) const {
    if (isNull(column)) {
        return std::nullopt;
    }
    return columnDouble(column);
}

// ---- Database ----------------------------------------------------------------

Database::Database(const std::string& path) {
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    if (sqlite3_open_v2(path.c_str(), &db_, flags, nullptr) != SQLITE_OK) {
        const std::string detail = db_ != nullptr ? sqlite3_errmsg(db_) : "out of memory";
        sqlite3_close(db_);
        db_ = nullptr;
        throw DatabaseError("Cannot open database '" + path + "': " + detail);
    }
    sqlite3_busy_timeout(db_, 5000);
    execute("PRAGMA foreign_keys = ON;");
    if (path != ":memory:") {
        // WAL gives crash safety and lets readers proceed while a write is in progress.
        execute("PRAGMA journal_mode = WAL;");
        execute("PRAGMA synchronous = NORMAL;");
    }
}

Database::~Database() {
    if (db_ != nullptr) {
        sqlite3_close_v2(db_);
    }
}

void Database::execute(const std::string& sql) {
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        const std::string detail = error != nullptr ? error : "unknown error";
        sqlite3_free(error);
        throw DatabaseError("SQL execution failed: " + detail);
    }
}

Statement Database::prepare(const std::string& sql) {
    return {db_, sql};
}

std::int64_t Database::lastInsertId() const {
    return sqlite3_last_insert_rowid(db_);
}

int Database::changes() const {
    return sqlite3_changes(db_);
}

int Database::userVersion() {
    Statement st = prepare("PRAGMA user_version;");
    return st.step() ? st.columnInt(0) : 0;
}

void Database::setUserVersion(int version) {
    // PRAGMA does not accept bound parameters; the value is an int, never user text.
    execute("PRAGMA user_version = " + std::to_string(version) + ";");
}

void Database::inTransaction(const std::function<void()>& work) {
    const std::string name = "sp" + std::to_string(++savepointDepth_);
    try {
        execute("SAVEPOINT " + name + ";");
    } catch (...) {
        --savepointDepth_;
        throw;
    }
    try {
        work();
        execute("RELEASE SAVEPOINT " + name + ";");
        --savepointDepth_;
    } catch (...) {
        try {
            execute("ROLLBACK TO SAVEPOINT " + name + ";");
            execute("RELEASE SAVEPOINT " + name + ";");
        } catch (...) {  // NOLINT(bugprone-empty-catch)
            // Deliberately swallowed: the original exception (rethrown below) explains the
            // failure; a secondary rollback error would hide it.
        }
        --savepointDepth_;
        throw;
    }
}

// ---- SchemaMigrator ----------------------------------------------------------

int SchemaMigrator::latestVersion() {
    return static_cast<int>(sizeof(generated::kMigrations) / sizeof(generated::kMigrations[0]));
}

void SchemaMigrator::migrate(Database& db) {
    const int current = db.userVersion();
    const int latest = latestVersion();
    if (current > latest) {
        throw DatabaseError("The database was created by a newer version of the LMS (schema " +
                            std::to_string(current) + "); this program supports schema " +
                            std::to_string(latest) + ".");
    }
    for (int version = current + 1; version <= latest; ++version) {
        db.inTransaction([&] {
            db.execute(generated::kMigrations[version - 1]);
            db.setUserVersion(version);
        });
    }
}

}  // namespace lms
