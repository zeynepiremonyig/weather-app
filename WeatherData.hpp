#pragma once

#include <string>

// This struct holds the weather information we want to show to the user.
// Every field has a safe default value, so the program can still behave
// predictably if parsing fails or if some API fields are missing.
struct WeatherData {
    std::string cityName = "Unknown city";
    std::string country = "Unknown country";
    double temperature = 0.0;
    std::string conditionText = "No data";
    int humidity = 0;
};