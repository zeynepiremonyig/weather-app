#include "ConsoleUi.hpp"
#include <iostream>

// -----------------------------------------------------------------------------
// ConsoleUi (implementation)
// -----------------------------------------------------------------------------
// What this translation unit does:
// 1) Centralizes every std::cout line used for menus and user feedback.
// 2) Keeps main.cpp free of raw formatting strings for the CLI.
//
// Why this exists:
// If we change how messages look (prefixes, spacing, language), we edit one place
// instead of hunting strings across networking and business logic.

// -----------------------------------------------------------------------------
// showLoginMenu
// -----------------------------------------------------------------------------
// What this function does:
// 1) Prints the public (logged-out) menu header and numbered options.
// 2) Ends with "Choice: " so the user knows where to type.
//
// Why this exists:
// main.cpp only decides flow; this file owns the exact text the beginner sees.
void ConsoleUi::showLoginMenu() {
    std::cout << "\n=== WEATHER APP LOGIN MENU ===" << std::endl;
    std::cout << "1. Register" << std::endl;
    std::cout << "2. Login" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "Choice: ";
}

// -----------------------------------------------------------------------------
// showDashboardMenu
// -----------------------------------------------------------------------------
// What this function does:
// 1) Shows the authenticated menu with the current username in the title.
// 2) Lists actions available after login (weather search / logout).
//
// Parameters:
// - username -> displayed in the banner so the user sees which account is active.
void ConsoleUi::showDashboardMenu(const std::string& username) {
    std::cout << "\n=== DASHBOARD (" << username << ") ===" << std::endl;
    std::cout << "1. Search Weather" << std::endl;
    std::cout << "2. Logout" << std::endl;
    std::cout << "Choice: ";
}

// -----------------------------------------------------------------------------
// showWeatherReport
// -----------------------------------------------------------------------------
// What this function does:
// 1) Takes a filled WeatherData object (already parsed from JSON).
// 2) Prints a simple framed block with city, country, temperature, condition, humidity.
//
// Why this exists:
// Parsing (WeatherParser) and fetching (WeatherClient) stay technical;
// this layer turns numbers/strings into a readable terminal report.
void ConsoleUi::showWeatherReport(const WeatherData& weatherData) {
    std::cout << "\n-----------------------------------" << std::endl;
    std::cout << " WEATHER REPORT FOR " << weatherData.cityName << std::endl;
    std::cout << " Country     : " << weatherData.country << std::endl;
    std::cout << " Temperature : " << weatherData.temperature << " C" << std::endl;
    std::cout << " Condition   : " << weatherData.conditionText << std::endl;
    std::cout << " Humidity    : " << weatherData.humidity << "%" << std::endl;
    std::cout << "-----------------------------------\n" << std::endl;
}

// -----------------------------------------------------------------------------
// showInfo
// -----------------------------------------------------------------------------
// What this function does:
// Prints a non-error status line with an [Info] prefix.
//
// Typical use:
// Success confirmations (registration done, welcome back, query logged, etc.).
void ConsoleUi::showInfo(const std::string& message) {
    std::cout << "[Info] " << message << std::endl;
}

// -----------------------------------------------------------------------------
// showWarning
// -----------------------------------------------------------------------------
// What this function does:
// Prints a softer alert than errors (operation partly succeeded or degraded).
//
// Example scenario from main:
// Weather displayed but database logging failed — user should still see data.
void ConsoleUi::showWarning(const std::string& message) {
    std::cout << "[Warning] " << message << std::endl;
}

// -----------------------------------------------------------------------------
// showError
// -----------------------------------------------------------------------------
// What this function does:
// Prints validation failures, bad menu choices, auth errors, and network/parse issues.
//
// Return value:
// None (void) — this is output-only; callers decide whether to continue the loop.
void ConsoleUi::showError(const std::string& message) {
    std::cout << "[Error] " << message << std::endl;
}
