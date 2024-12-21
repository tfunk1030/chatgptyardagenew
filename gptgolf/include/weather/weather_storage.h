#pragma once

#include "weather_data.h"
#include <string>
#include <vector>
#include <optional>
#include <memory>

// Forward declare sqlite3 to avoid direct dependency
struct sqlite3;

namespace gptgolf {
namespace weather {

class WeatherStorage {
public:
    WeatherStorage();
    ~WeatherStorage();

    // Initialize storage with database file
    bool initialize(const std::string& dbPath);

    // Store weather data
    bool storeWeatherData(double latitude, double longitude, const WeatherData& data);

    // Retrieve weather data
    std::optional<WeatherData> getWeatherData(double latitude, double longitude);

    // Get typical weather conditions for location and time of year
    std::optional<WeatherData> getTypicalWeather(double latitude, double longitude);

    // Store typical weather patterns
    bool storeTypicalWeather(double latitude, double longitude, 
                           int month, const WeatherData& data);

    // Clear old data
    void clearOldData(std::time_t olderThan);

    // Check if we have recent data for location
    bool hasRecentData(double latitude, double longitude, int maxAgeMinutes = 60);

    // Get nearest available weather data
    std::optional<WeatherData> getNearestWeatherData(double latitude, double longitude, 
                                                   double maxDistanceKm = 10.0);

    // Statistics
    struct WeatherStats {
        double avgTemperature;
        double avgHumidity;
        double avgPressure;
        double avgWindSpeed;
        std::vector<double> windDirectionFrequency; // 0-360 degrees in 10-degree bins
    };

    // Get historical statistics for location
    std::optional<WeatherStats> getHistoricalStats(double latitude, double longitude,
                                                 int month);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    // Helper methods
    bool initializeTables();
    double calculateDistance(double lat1, double lon1, double lat2, double lon2);
    int getLocationBin(double latitude, double longitude);
};

// Singleton instance
class WeatherStorageService {
public:
    static WeatherStorageService& getInstance() {
        static WeatherStorageService instance;
        return instance;
    }

    bool initialize(const std::string& dbPath) {
        return storage.initialize(dbPath);
    }

    WeatherStorage& getStorage() { return storage; }

private:
    WeatherStorageService() = default;
    WeatherStorage storage;

    // Prevent copying
    WeatherStorageService(const WeatherStorageService&) = delete;
    WeatherStorageService& operator=(const WeatherStorageService&) = delete;
};

} // namespace weather
} // namespace gptgolf
