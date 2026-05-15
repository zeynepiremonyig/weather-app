#pragma once

#include <string>
#include <sqlite3.h>

// Database class is the only place that talks to SQLite directly.
// Main/Auth code should ask this class for data instead of writing SQL there.
// This is a common "single responsibility" design practice.
class Database {
private:
    sqlite3* db;

public:
    // Opens (or creates) the database file.
    explicit Database(const std::string& dbPath);

    // Closes the database connection.
    ~Database();

    // Inserts a new user record using a prepared statement.
    bool registerUser(const std::string& username, const std::string& passwordHash);

    // Checks credentials and returns the user ID when match is found.
    int authenticateUser(const std::string& username, const std::string& passwordHash);

    // Reads user ID and stored password data for authentication workflows
    // that need custom verification logic in upper layers.
    bool getUserAuthData(const std::string& username, int& userId, std::string& storedPasswordData);

    // Executes raw SQL text (for schema/setup statements).
    // For user-input values we prefer prepared statements (safer).
    bool execute(const std::string& sqlQuery);

    // Saves one weather query row for the given user.
    bool logQuery(int userId, const std::string& city, const std::string& resultSummary);
};