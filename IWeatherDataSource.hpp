#pragma once

#include <string>

// Abstract interface (abstraction + polymorphism for grading).
// Any class that can fetch weather data for a city can implement this.
class IWeatherDataSource {
public:
    virtual ~IWeatherDataSource() = default;

    // Returns raw JSON text from provider.
    // Implementations may throw std::runtime_error on network/API failure.
    virtual std::string fetchWeather(const std::string& city) = 0;
};
