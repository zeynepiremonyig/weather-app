#include "SessionManager.hpp"

// Constructor starts with "no active user" state.
SessionManager::SessionManager() : currentUser{}, loggedIn(false) {}

// loginUser() stores active user info in memory.
void SessionManager::loginUser(int id, const std::string& username) {
    currentUser.id = id;
    currentUser.username = username;
    loggedIn = true;
}

// logoutUser() clears active user info.
void SessionManager::logoutUser() {
    loggedIn = false;
    currentUser = User{};
}

// isLoggedIn() is used by main menu flow.
bool SessionManager::isLoggedIn() const {
    return loggedIn;
}

// getCurrentUser() returns a copy of current user data.
User SessionManager::getCurrentUser() const {
    return currentUser;
}