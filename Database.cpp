#include "Database.hpp"
#include <iostream>
#include <stdexcept>

// Constructor:
// - Opens existing SQLite file or creates a new file if missing.
// - Throws runtime_error if open fails (fail-fast behavior).
Database::Database(const std::string& dbPath) : db(nullptr) {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        const std::string message = "Failed to open database: " + std::string(sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        throw std::runtime_error(message);
    }

    // SQLite keeps foreign-key checks disabled by default.
    // Enabling this protects referential integrity (e.g., QueryLogs.user_id must reference Users.id).
    if (!execute("PRAGMA foreign_keys = ON;")) {
        sqlite3_close(db);
        db = nullptr;
        throw std::runtime_error("Failed to enable SQLite foreign key enforcement.");
    }
}

// Destructor:
// - Releases database connection when Database object leaves scope.
Database::~Database() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

// execute()
// ---------
// Runs generic SQL text (for example schema creation).
// Returns true on success, false on SQL error.
bool Database::execute(const std::string& sqlQuery) {
    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db, sqlQuery.c_str(), nullptr, nullptr, &errorMessage);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL execution error: " << (errorMessage != nullptr ? errorMessage : "Unknown error") << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}

// logQuery()
// ----------
// Saves one weather search event into QueryLogs table.
// Uses prepared statements to avoid SQL injection.
bool Database::logQuery(int userId, const std::string& city, const std::string& resultSummary) {
    const std::string sql = "INSERT INTO QueryLogs (user_id, city, result_summary) VALUES (?, ?, ?);";

    sqlite3_stmt* statement = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare query-log statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(statement, 1, userId);
    sqlite3_bind_text(statement, 2, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, resultSummary.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(statement);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute query-log statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }

    sqlite3_finalize(statement);
    return true;
}

// registerUser()
// --------------
// Inserts a new user row.
// passwordHash is expected to be already hashed by AuthService.
bool Database::registerUser(const std::string& username, const std::string& passwordHash) {
    const std::string sql = "INSERT INTO Users (username, password_hash) VALUES (?, ?);";
    sqlite3_stmt* statement = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare register statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (rc == SQLITE_DONE) {
        return true;
    } else {
        std::cerr << "Registration failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

// authenticateUser()
// ------------------
// Legacy helper that checks username + password_hash directly in SQL.
// Returns user ID if found, otherwise -1.
// (Still kept for compatibility, though current login path uses getUserAuthData.)
int Database::authenticateUser(const std::string& username, const std::string& passwordHash) {
    const std::string sql = "SELECT id FROM Users WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt* statement = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare login statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);

    int userId = -1;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        userId = sqlite3_column_int(statement, 0);
    }

    sqlite3_finalize(statement);
    return userId;
}

// getUserAuthData()
// -----------------
// Fetches user ID and stored password data by username.
// This supports modern verification logic in AuthService
// (for example salt$hash parsing).
bool Database::getUserAuthData(const std::string& username, int& userId, std::string& storedPasswordData) {
    const std::string sql = "SELECT id, password_hash FROM Users WHERE username = ?;";
    sqlite3_stmt* statement = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare auth-data statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(statement, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        userId = sqlite3_column_int(statement, 0);
        const unsigned char* text = sqlite3_column_text(statement, 1);
        storedPasswordData = (text != nullptr) ? reinterpret_cast<const char*>(text) : "";
        found = true;
    }

    sqlite3_finalize(statement);
    return found;
}