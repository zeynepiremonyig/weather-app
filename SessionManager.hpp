#pragma once
#include <string>
#include "User.hpp"

// SessionManager keeps track of authentication state during runtime.
// It does not write data to disk; it only stores current session values in memory.
class SessionManager {
private:
    User currentUser;
    bool loggedIn;

public:
    // Creates an empty session. No user is logged in at startup.
    SessionManager();

    // Marks the session as active and stores user identity.
    void loginUser(int id, const std::string& username);

    // Clears session data and marks user as logged out.
    void logoutUser();

    // Returns true when a user is currently authenticated.
    bool isLoggedIn() const;

    // Returns a copy of the active user object.
    User getCurrentUser() const;
};