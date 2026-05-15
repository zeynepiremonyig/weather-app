#pragma once

#include <string>
#include "Database.hpp"

// AuthService contains authentication business logic.
// Database stores/retrieves rows, but AuthService decides HOW to:
// - hash passwords
// - format password storage value
// - verify credentials (including legacy compatibility)
class AuthService {
private:
    Database& db;

    // Creates SHA-256 hash from plainPassword + salt.
    std::string hashPasswordWithSalt(const std::string& plainPassword, const std::string& salt);

    // Creates a random salt value for each new user.
    std::string generateSalt() const;

public:
    // Binds this service to a live Database object.
    AuthService(Database& database);

    // Creates a new account. Returns true on success.
    bool registerAccount(const std::string& username, const std::string& plainPassword);

    // Checks credentials. Returns user ID on success, or -1 on failure.
    int login(const std::string& username, const std::string& plainPassword);
};