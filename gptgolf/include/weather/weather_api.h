#ifndef WEATHER_API_H
#define WEATHER_API_H

#include "weather_data.h"
#include "weather_storage.h"
#include <string>
#include <memory>
#include <functional>

namespace gptgolf {
namespace weather {

class WeatherAPI {
public:
    explicit WeatherAPI(WeatherStorage& weatherStorage);
    ~WeatherAPI();

    // Initialize the API with credentials and mode
    bool initialize(const std::string& apiKey, bool useOfflineMode = false);

    // Set offline mode
    void setOfflineMode(bool enabled);

    // Check if offline mode is active
    bool isOfflineMode() const;

    // Get current weather data for a location
    bool getCurrentWeather(double latitude, double longitude, WeatherData& data);

    // Set callback for error handling
    void setErrorCallback(std::function<void(const std::string&)> callback);

    // Check if the API is ready
    bool isInitialized() const;

    // Constants (static constexpr for in-class initialization)
    static constexpr int MAX_CACHE_AGE_MINUTES = 60;
    static constexpr double MAX_DISTANCE_KM = 10.0;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl; // PIMPL idiom to hide implementation details

    // Storage for offline data
    WeatherStorage& storage;

    // API configuration
    std::string apiKey;
    bool initialized;
    std::function<void(const std::string&)> errorCallback;
    bool offlineMode;

    // Internal helper methods
    bool fetchFromAPI(double latitude, double longitude, WeatherData& data);
    bool parseWeatherResponse(const std::string& response, WeatherData& data);
    bool getOfflineWeather(double latitude, double longitude, WeatherData& data);
};

// Singleton instance for global access
class WeatherService {
public:
    static WeatherService& getInstance() {
        static WeatherService instance;
        return instance;
    }

    bool initialize(const std::string& apiKey, bool useOfflineMode = false) {
        return api.initialize(apiKey, useOfflineMode);
    }

    WeatherAPI& getAPI() { return api; }

private:
    WeatherService() : api(WeatherStorageService::getInstance().getStorage()) {}
    WeatherAPI api;

    // Prevent copying
    WeatherService(const WeatherService&) = delete;
    WeatherService& operator=(const WeatherService&) = delete;
};

} // namespace weather
} // namespace gptgolf

#endif // WEATHER_API_H
