#pragma once

#include <string>
#include "IWeatherDataSource.hpp"

// WeatherClient handles ONLY remote API communication.
// It does not parse JSON and it does not print menu text.
// Keeping responsibilities separated makes code easier to understand and test.
class WeatherClient : public IWeatherDataSource {
private:
    std::string apiKey;

    // Utility function used before building request URL.
    // Converts characters such as:
    //   " " -> "%20"
    //   "+" -> "%2B"
    // so city names are safe inside URL query string.
    static std::string urlEncode(const std::string& value);

public:
    // Stores the API key for future requests.
    explicit WeatherClient(std::string key);

    // Sends HTTPS request and returns server JSON body as plain text.
    // Throws std::runtime_error in these cases:
    // - DNS/TCP/TLS problems
    // - non-200 HTTP status code
    // - fatal TLS shutdown error
    std::string fetchWeather(const std::string& city) override;
};