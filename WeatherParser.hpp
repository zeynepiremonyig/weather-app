#pragma once

#include <string>
#include "WeatherData.hpp"

// WeatherParser converts JSON text into WeatherData object.
// It is intentionally independent from networking and database layers.
// This separation keeps each class simple and easier for beginners to reason about.
class WeatherParser {
public:
    // Tries to parse the API JSON string.
    // - On success: returns true and fills outputData.
    // - On failure: returns false and writes a friendly message to errorMessage.
    // Why this pattern?
    // Returning bool + error text is easier for beginners than exception-only flow.
    static bool parse(
        const std::string& rawJsonString,
        WeatherData& outputData,
        std::string& errorMessage
    );
};