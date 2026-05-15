#pragma once

#include <string>
#include "WeatherData.hpp"

// ConsoleUi owns all terminal printing responsibilities.
// Keeping output here separates display logic from networking and parsing logic.
class ConsoleUi {
public:
    static void showLoginMenu();
    static void showDashboardMenu(const std::string& username);
    static void showWeatherReport(const WeatherData& weatherData);
    static void showInfo(const std::string& message);
    static void showWarning(const std::string& message);
    static void showError(const std::string& message);
};
