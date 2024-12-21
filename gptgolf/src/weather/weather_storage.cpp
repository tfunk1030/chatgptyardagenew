#include "weather/weather_storage.h"
#include <cmath>
#include <sstream>
#include <vector>
#include <iomanip>
#include <sqlite3.h>

namespace gptgolf {
namespace weather {

class WeatherStorage::Impl {
public:
    Impl() : db(nullptr) {}
    ~Impl() {
        if (db) sqlite3_close(db);
    }

    sqlite3* db;
};

WeatherStorage::WeatherStorage() : pImpl(new Impl()) {}
WeatherStorage::~WeatherStorage() = default;

bool WeatherStorage::initialize(const std::string& dbPath) {
    int rc = sqlite3_open(dbPath.c_str(), &pImpl->db);
    if (rc) {
        return false;
    }
    return initializeTables();
}

bool WeatherStorage::initializeTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS weather_data (
            latitude REAL,
            longitude REAL,
            temperature REAL,
            humidity REAL,
            pressure REAL,
            wind_speed REAL,
            wind_direction REAL,
            precipitation REAL,
            altitude REAL,
            timestamp INTEGER,
            PRIMARY KEY (latitude, longitude, timestamp)
        );

        CREATE TABLE IF NOT EXISTS typical_weather (
            latitude REAL,
            longitude REAL,
            month INTEGER,
            temperature REAL,
            humidity REAL,
            pressure REAL,
            wind_speed REAL,
            wind_direction REAL,
            precipitation REAL,
            altitude REAL,
            PRIMARY KEY (latitude, longitude, month)
        );

        CREATE TABLE IF NOT EXISTS wind_patterns (
            latitude REAL,
            longitude REAL,
            speed REAL,
            direction REAL,
            gust_speed REAL,
            temperature REAL,
            pressure REAL,
            timestamp INTEGER,
            hour_of_day INTEGER,
            PRIMARY KEY (latitude, longitude, timestamp)
        );

        CREATE TABLE IF NOT EXISTS terrain_data (
            latitude REAL,
            longitude REAL,
            land_use TEXT,
            elevation REAL,
            roughness_length REAL,
            roughness_variation REAL,
            is_complex INTEGER,
            last_updated INTEGER,
            PRIMARY KEY (latitude, longitude)
        );

        CREATE INDEX IF NOT EXISTS idx_wind_patterns_location_time 
        ON wind_patterns(latitude, longitude, hour_of_day);

        CREATE INDEX IF NOT EXISTS idx_terrain_location 
        ON terrain_data(latitude, longitude);

        CREATE INDEX IF NOT EXISTS idx_weather_location 
        ON weather_data(latitude, longitude);

        CREATE INDEX IF NOT EXISTS idx_weather_timestamp 
        ON weather_data(timestamp);
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(pImpl->db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool WeatherStorage::storeWeatherData(double latitude, double longitude, const WeatherData& data) {
    const char* sql = R"(
        INSERT OR REPLACE INTO weather_data 
        (latitude, longitude, temperature, humidity, pressure, wind_speed, 
         wind_direction, precipitation, altitude, timestamp)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_double(stmt, 3, data.temperature);
    sqlite3_bind_double(stmt, 4, data.humidity);
    sqlite3_bind_double(stmt, 5, data.pressure);
    sqlite3_bind_double(stmt, 6, data.windSpeed);
    sqlite3_bind_double(stmt, 7, data.windDirection);
    sqlite3_bind_double(stmt, 8, data.precipitation);
    sqlite3_bind_double(stmt, 9, data.altitude);
    sqlite3_bind_int64(stmt, 10, data.timestamp);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::optional<WeatherData> WeatherStorage::getWeatherData(double latitude, double longitude) {
    const char* sql = R"(
        SELECT * FROM weather_data 
        WHERE latitude = ? AND longitude = ?
        ORDER BY timestamp DESC LIMIT 1;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);

    WeatherData data;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        data.temperature = sqlite3_column_double(stmt, 2);
        data.humidity = sqlite3_column_double(stmt, 3);
        data.pressure = sqlite3_column_double(stmt, 4);
        data.windSpeed = sqlite3_column_double(stmt, 5);
        data.windDirection = sqlite3_column_double(stmt, 6);
        data.precipitation = sqlite3_column_double(stmt, 7);
        data.altitude = sqlite3_column_double(stmt, 8);
        data.timestamp = sqlite3_column_int64(stmt, 9);
        sqlite3_finalize(stmt);
        return data;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool WeatherStorage::hasRecentData(double latitude, double longitude, int maxAgeMinutes) {
    const char* sql = R"(
        SELECT COUNT(*) FROM weather_data 
        WHERE latitude = ? AND longitude = ?
        AND timestamp > ? LIMIT 1;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    std::time_t cutoff = std::time(nullptr) - (maxAgeMinutes * 60);
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_int64(stmt, 3, cutoff);

    bool hasRecent = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        hasRecent = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return hasRecent;
}

double WeatherStorage::calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Earth's radius in km
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(lat1 * M_PI/180.0) * cos(lat2 * M_PI/180.0) *
               sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}

std::optional<WeatherData> WeatherStorage::getNearestWeatherData(double latitude, double longitude, double maxDistanceKm) {
    const char* sql = R"(
        SELECT *, (
            6371 * acos(
                cos(radians(?)) * cos(radians(latitude)) *
                cos(radians(longitude) - radians(?)) +
                sin(radians(?)) * sin(radians(latitude))
            )
        ) as distance
        FROM weather_data
        WHERE timestamp > ?
        HAVING distance < ?
        ORDER BY distance, timestamp DESC
        LIMIT 1;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    std::time_t cutoff = std::time(nullptr) - 3600; // Last hour
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_double(stmt, 3, latitude);
    sqlite3_bind_int64(stmt, 4, cutoff);
    sqlite3_bind_double(stmt, 5, maxDistanceKm);

    WeatherData data;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        data.temperature = sqlite3_column_double(stmt, 2);
        data.humidity = sqlite3_column_double(stmt, 3);
        data.pressure = sqlite3_column_double(stmt, 4);
        data.windSpeed = sqlite3_column_double(stmt, 5);
        data.windDirection = sqlite3_column_double(stmt, 6);
        data.precipitation = sqlite3_column_double(stmt, 7);
        data.altitude = sqlite3_column_double(stmt, 8);
        data.timestamp = sqlite3_column_int64(stmt, 9);
        sqlite3_finalize(stmt);
        return data;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

void WeatherStorage::clearOldData(std::time_t olderThan) {
    const char* sql = "DELETE FROM weather_data WHERE timestamp < ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return;

    sqlite3_bind_int64(stmt, 1, olderThan);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

bool WeatherStorage::storeTypicalWeather(double latitude, double longitude, 
                                       int month, const WeatherData& data) {
    const char* sql = R"(
        INSERT OR REPLACE INTO typical_weather 
        (latitude, longitude, month, temperature, humidity, pressure, 
         wind_speed, wind_direction, precipitation, altitude)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_int(stmt, 3, month);
    sqlite3_bind_double(stmt, 4, data.temperature);
    sqlite3_bind_double(stmt, 5, data.humidity);
    sqlite3_bind_double(stmt, 6, data.pressure);
    sqlite3_bind_double(stmt, 7, data.windSpeed);
    sqlite3_bind_double(stmt, 8, data.windDirection);
    sqlite3_bind_double(stmt, 9, data.precipitation);
    sqlite3_bind_double(stmt, 10, data.altitude);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::optional<WeatherData> WeatherStorage::getTypicalWeather(double latitude, double longitude) {
    const char* sql = R"(
        SELECT * FROM typical_weather 
        WHERE latitude = ? AND longitude = ?
        AND month = ?;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    // Get current month (1-12)
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    int currentMonth = ltm->tm_mon + 1;

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_int(stmt, 3, currentMonth);

    WeatherData data;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        data.temperature = sqlite3_column_double(stmt, 3);
        data.humidity = sqlite3_column_double(stmt, 4);
        data.pressure = sqlite3_column_double(stmt, 5);
        data.windSpeed = sqlite3_column_double(stmt, 6);
        data.windDirection = sqlite3_column_double(stmt, 7);
        data.precipitation = sqlite3_column_double(stmt, 8);
        data.altitude = sqlite3_column_double(stmt, 9);
        data.timestamp = now;
        sqlite3_finalize(stmt);
        return data;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<WeatherStorage::WeatherStats> WeatherStorage::getHistoricalStats(
    double latitude, double longitude, int month) {
    const char* sql = R"(
        SELECT 
            AVG(temperature) as avg_temp,
            AVG(humidity) as avg_hum,
            AVG(pressure) as avg_press,
            AVG(wind_speed) as avg_wind
        FROM weather_data 
        WHERE latitude = ? AND longitude = ?
        AND strftime('%m', datetime(timestamp, 'unixepoch')) = ?;
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    std::stringstream monthStr;
    monthStr << std::setw(2) << std::setfill('0') << month;
    
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, monthStr.str().c_str(), -1, SQLITE_STATIC);

    WeatherStats stats;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        stats.avgTemperature = sqlite3_column_double(stmt, 0);
        stats.avgHumidity = sqlite3_column_double(stmt, 1);
        stats.avgPressure = sqlite3_column_double(stmt, 2);
        stats.avgWindSpeed = sqlite3_column_double(stmt, 3);
        
        // Initialize wind direction frequency bins
        stats.windDirectionFrequency.resize(36, 0.0); // 36 bins of 10 degrees each
        sqlite3_finalize(stmt);
        
        // Get wind direction distribution in separate query
        const char* windSql = R"(
            SELECT wind_direction, COUNT(*) as freq
            FROM weather_data
            WHERE latitude = ? AND longitude = ?
            AND strftime('%m', datetime(timestamp, 'unixepoch')) = ?
            GROUP BY CAST((wind_direction / 10) AS INT);
        )";
        
        rc = sqlite3_prepare_v2(pImpl->db, windSql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) return stats;
        
        sqlite3_bind_double(stmt, 1, latitude);
        sqlite3_bind_double(stmt, 2, longitude);
        sqlite3_bind_text(stmt, 3, monthStr.str().c_str(), -1, SQLITE_STATIC);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            double direction = sqlite3_column_double(stmt, 0);
            int freq = sqlite3_column_int(stmt, 1);
            int bin = static_cast<int>(direction / 10.0) % 36;
            stats.windDirectionFrequency[bin] = freq;
        }
        
        sqlite3_finalize(stmt);
        return stats;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

} // namespace weather
} // namespace gptgolf
