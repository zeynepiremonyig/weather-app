#include "AuthService.hpp"
#include "picosha2.h"
#include <random>
#include <sstream>

// AuthService is a thin business-logic layer over Database.
// It handles password hashing and credential verification.
AuthService::AuthService(Database& database) : db(database) {}

std::string AuthService::hashPasswordWithSalt(
    const std::string& plainPassword,
    const std::string& salt
) {
    // Salted hashing:
    // same password + different salt => different final hash.
    std::string hashHexStr;
    picosha2::hash256_hex_string(salt + plainPassword, hashHexStr);
    return hashHexStr;
}

std::string AuthService::generateSalt() const {
    // We generate 16 random bytes and store them as a hex string.
    // Each user gets a different salt, so same passwords do not produce same hashes.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::ostringstream saltStream;

    for (int i = 0; i < 16; ++i) {
        const int value = dist(gen);
        const char high = "0123456789abcdef"[(value >> 4) & 0x0F];
        const char low = "0123456789abcdef"[value & 0x0F];
        saltStream << high << low;
    }

    return saltStream.str();
}

bool AuthService::registerAccount(const std::string& username, const std::string& plainPassword) {
    // For every new account:
    // 1) create random salt
    // 2) compute salted hash
    // 3) store both values in one DB field using "salt$hash" format
    const std::string salt = generateSalt();
    const std::string hash = hashPasswordWithSalt(plainPassword, salt);

    // Stored format: "<salt>$<hash>"
    // This keeps DB schema unchanged and still lets us use per-user salt.
    const std::string storedPasswordData = salt + "$" + hash;
    return db.registerUser(username, storedPasswordData);
}

int AuthService::login(const std::string& username, const std::string& plainPassword) {
    // Read DB value first, then verify in C++.
    // This allows flexible formats (new salted format + legacy format).
    int userId = -1;
    std::string storedPasswordData;
    if (!db.getUserAuthData(username, userId, storedPasswordData)) {
        return -1;
    }

    const std::size_t separatorPos = storedPasswordData.find('$');
    if (separatorPos == std::string::npos) {
        // Backward compatibility:
        // Old accounts may store only plain SHA-256(password) without salt.
        // For those records, compare directly with the legacy hash format.
        std::string legacyHash;
        picosha2::hash256_hex_string(plainPassword, legacyHash);
        return (legacyHash == storedPasswordData) ? userId : -1;
    }

    // New format path: split to "salt" and "hash", then recompute.
    const std::string salt = storedPasswordData.substr(0, separatorPos);
    const std::string storedHash = storedPasswordData.substr(separatorPos + 1);
    const std::string inputHash = hashPasswordWithSalt(plainPassword, salt);

    return (inputHash == storedHash) ? userId : -1;
}