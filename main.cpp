#include <iostream> 
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem> // for file and folder paths
#include <cstdlib>  // std::getenv to read environment variables
#include <algorithm>  // find_if_not in the trim function
#include <cctype>  // std::isspace to check spaces

#include "Database.hpp"
#include "AuthService.hpp"
#include "SessionManager.hpp"
#include "WeatherClient.hpp"
#include "WeatherParser.hpp"
#include "ConsoleUi.hpp"

namespace fs = std::filesystem;

// This function prepares the database when the program starts.
bool initializeDatabase(Database& db, const fs::path& schemaPath) {
    std::ifstream schemaFile(schemaPath); // trying to open the shema.sql file
    
    if (schemaFile.is_open()) {
        std::stringstream buffer; // This object helps us read the whole file into one string.
        buffer << schemaFile.rdbuf();

        if (db.execute(buffer.str())) { // db.execute(...) will run the SQL commands.
            std::cout << "[System] Database initialized successfully." << std::endl;
            return true;
        }
        std::cerr << "[System Error] Failed to execute schema SQL." << std::endl;
        return false;
    } else {
        std::cerr << "[System Error] Could not open schema file: " << schemaPath << std::endl;
        return false;
    }
}

// Removes spaces at the beginning and end.
// Example: "  London  " -> "London"
std::string trim(const std::string& input) {
    // Find the first character that is NOT a space.
    const auto first = std::find_if_not(input.begin(), input.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0; // if "ch" is a space, it returns true
    });
    if (first == input.end()) {
        return ""; // if the input is entirely space
    }

    const auto last = std::find_if_not(input.rbegin(), input.rend(), [](unsigned char ch) { // reverse iterators
        return std::isspace(ch) != 0; 
    }).base(); // .base() converts to the normal iterator pointing after the last non-space character

    return std::string(first, last);
}

// This function checks if user input is acceptable. (not empty and not too long)
bool isValidInput(const std::string& value, std::size_t maxLength) {
    const std::string cleaned = trim(value);
    return !cleaned.empty() && cleaned.size() <= maxLength;
}

int main() {
    try {
        // SECURITY NOTE:
        // The API key is saved as an environment variable in the computer
        const char* apiKeyEnv = std::getenv("WEATHER_API_KEY");
        if (apiKeyEnv == nullptr || std::string(apiKeyEnv).empty()) {
            std::cerr << "[Startup Error] WEATHER_API_KEY is not set." << std::endl;
            std::cerr << "Please set your key before starting the app." << std::endl;
            return 1;
        }

        // PATH STRATEGY:
        // The program may run from different folders. So we try current folder first, then parent folder.
        const fs::path cwd = fs::current_path(); // current working directory
        // Find schema.sql, file contains SQL commands for creating database tables.
        const fs::path schemaPath =
            fs::exists(cwd / "schema.sql") ? (cwd / "schema.sql") : (cwd.parent_path() / "schema.sql");
        // Find or create the weather.db database path, this is the SQLite database file.
        const fs::path databasePath =
            fs::exists(cwd / "weather.db") ? (cwd / "weather.db") : (cwd.parent_path() / "weather.db");

        // This object talks to the SQLite database.
        Database db(databasePath.string());
        if (!initializeDatabase(db, schemaPath)) { // Run schema.sql to create tables if needed.
            return 1;
        }

        AuthService auth(db); // auth object is for register and login operations, it uses database
        SessionManager session; // remembers who is currently logged in 
        WeatherClient weatherClient(apiKeyEnv); // connects to the api 
        // Polymorphism in action:
        // main layer depends on abstract interface, not concrete class.
        IWeatherDataSource& weatherDataSource = weatherClient;

        bool appRunning = true;

        // MAIN LOOP:
        // Keep showing menus until user picks Exit.
        // The menu changes depending on whether the user is logged in.
        while (appRunning) {
            if (!session.isLoggedIn()) {
                // Logged-out menu: Register / Login / Exit
                ConsoleUi::showLoginMenu();

                std::string choice;
                std::getline(std::cin, choice);

                if (choice == "1") {
                    // Registration flow:
                    // - Ask username/password
                    // - Validate not empty
                    // - Save through AuthService (which hashes password)
                    std::string username;
                    std::string password;
                    std::cout << "Choose a username: ";
                    std::getline(std::cin, username);
                    std::cout << "Choose a password: ";
                    std::getline(std::cin, password);
                    username = trim(username);
                    password = trim(password);

                    if (!isValidInput(username, 32) || !isValidInput(password, 64)) {
                        ConsoleUi::showError("Username/password must be non-empty and within length limits.");
                        continue;
                    }

                    if (auth.registerAccount(username, password)) {
                        ConsoleUi::showInfo("Registration complete! You can now log in.");
                    } else {
                        ConsoleUi::showError("Registration failed. Username might be taken.");
                    }
                } else if (choice == "2") {
                    // Login flow:
                    // - Ask credentials
                    // - Validate not empty
                    // - Ask AuthService to verify credentials
                    std::string username;
                    std::string password;
                    std::cout << "Enter username: ";
                    std::getline(std::cin, username);
                    std::cout << "Enter password: ";
                    std::getline(std::cin, password);
                    username = trim(username);
                    password = trim(password);

                    if (!isValidInput(username, 32) || !isValidInput(password, 64)) {
                        ConsoleUi::showError("Username/password must be non-empty and within length limits.");
                        continue;
                    }

                    int userId = auth.login(username, password);
                    if (userId != -1) {
                        ConsoleUi::showInfo("Welcome back, " + username + "!");
                        session.loginUser(userId, username);
                    } else {
                        ConsoleUi::showError("Invalid username or password.");
                    }
                } else if (choice == "3") {
                    appRunning = false;
                    ConsoleUi::showInfo("Goodbye!");
                } else {
                    ConsoleUi::showError("Invalid menu option. Please choose 1, 2, or 3.");
                }
            } else {
                // Logged-in menu: Search Weather / Logout
                ConsoleUi::showDashboardMenu(session.getCurrentUser().username);

                std::string choice;
                std::getline(std::cin, choice);

                if (choice == "1") {
                    // Weather search flow:
                    // - Read city
                    // - Call remote API
                    // - Parse JSON
                    // - Print result
                    // - Store a summary in DB logs
                    std::cout << "Enter city name: ";
                    std::string city;
                    std::getline(std::cin, city);
                    city = trim(city);

                    if (!isValidInput(city, 80)) {
                        ConsoleUi::showError("City must be non-empty and shorter than 81 characters.");
                        continue;
                    }

                    ConsoleUi::showInfo("Fetching weather data...");

                    try {
                        // Network call can throw (DNS, TLS, HTTP error, etc.),
                        // so we keep it inside try/catch.
                        const std::string rawData = weatherDataSource.fetchWeather(city);
                        WeatherData parsedData;
                        std::string parseError;

                        if (!WeatherParser::parse(rawData, parsedData, parseError)) {
                            ConsoleUi::showError("Could not read weather data: " + parseError);
                            continue;
                        }

                        ConsoleUi::showWeatherReport(parsedData);

                        // Save a short result text for query history.
                        const std::string summary =
                            std::to_string(parsedData.temperature) + " C, " + parsedData.conditionText;
                        if (db.logQuery(session.getCurrentUser().id, city, summary)) {
                            ConsoleUi::showInfo("Query securely logged to database.");
                        } else {
                            ConsoleUi::showWarning("Weather shown, but query could not be logged.");
                        }
                    } catch (const std::exception& e) {
                        ConsoleUi::showError("Weather request failed: " + std::string(e.what()));
                    }
                } else if (choice == "2") {
                    session.logoutUser();
                    ConsoleUi::showInfo("Logged out successfully.");
                } else {
                    ConsoleUi::showError("Invalid menu option. Please choose 1 or 2.");
                }
            }
        }

    } catch(const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}