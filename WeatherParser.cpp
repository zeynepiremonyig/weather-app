#include "WeatherParser.hpp"
#include <nlohmann/json.hpp>
#include <exception>

// parse()
// ------
// Input:
//   rawJsonString -> raw JSON text from Weather API
// Output:
//   outputData    -> filled WeatherData on success
//   errorMessage  -> human-readable message on failure
//
// Returns:
//   true  -> parse success
//   false -> parse/validation failed
bool WeatherParser::parse(
    const std::string& rawJsonString,
    WeatherData& outputData,
    std::string& errorMessage
) {
    try {
        // Convert raw JSON text into a structured JSON object.
        const nlohmann::json jsonData = nlohmann::json::parse(rawJsonString);

        // Validate high-level keys before reading nested values.
        // This avoids exceptions for completely unexpected payloads.
        if (!jsonData.contains("location") || !jsonData.contains("current")) {
            errorMessage = "API response is missing location/current sections.";
            return false;
        }

        // Read fields with value(..., default) so missing fields still produce
        // a safe fallback instead of crashing.
        outputData.cityName = jsonData.at("location").value("name", "Unknown city");
        outputData.country = jsonData.at("location").value("country", "Unknown country");
        outputData.temperature = jsonData.at("current").value("temp_c", 0.0);
        outputData.humidity = jsonData.at("current").value("humidity", 0);

        // condition is nested under current.condition.text.
        // We check each level before reading.
        if (jsonData.at("current").contains("condition")) {
            outputData.conditionText = jsonData.at("current").at("condition").value("text", "No data");
        } else {
            outputData.conditionText = "No data";
        }

        return true;
    } catch (const nlohmann::json::exception& e) {
        errorMessage = std::string("JSON parsing failed: ") + e.what();
        return false;
    } catch (const std::exception& e) {
        errorMessage = std::string("Unexpected parser error: ") + e.what();
        return false;
    }
}