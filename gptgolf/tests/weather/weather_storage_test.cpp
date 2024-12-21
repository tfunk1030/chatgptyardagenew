#include <gtest/gtest.h>
#include "weather/weather_storage.h"
#include <filesystem>
#include <ctime>

class WeatherStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use temporary database file
        dbPath = "test_weather.db";
        storage.initialize(dbPath);
    }

    void TearDown() override {
        // Clean up test database
        std::filesystem::remove(dbPath);
    }

    WeatherData createTestData(double temp = 20.0, double wind = 5.0) {
        WeatherData data;
        data.temperature = temp;
        data.humidity = 65.0;
        data.pressure = 1013.25;
        data.windSpeed = wind;
        data.windDirection = 180.0;
        data.precipitation = 0.0;
        data.altitude = 100.0;
        data.timestamp = std::time(nullptr);
        return data;
    }

    std::string dbPath;
    WeatherStorage storage;
};

TEST_F(WeatherStorageTest, StoreAndRetrieveData) {
    double lat = 40.7128;
    double lon = -74.0060;
    WeatherData testData = createTestData();

    // Store data
    EXPECT_TRUE(storage.storeWeatherData(lat, lon, testData));

    // Retrieve data
    auto retrieved = storage.getWeatherData(lat, lon);
    ASSERT_TRUE(retrieved.has_value());

    // Verify data
    EXPECT_DOUBLE_EQ(retrieved->temperature, testData.temperature);
    EXPECT_DOUBLE_EQ(retrieved->humidity, testData.humidity);
    EXPECT_DOUBLE_EQ(retrieved->pressure, testData.pressure);
    EXPECT_DOUBLE_EQ(retrieved->windSpeed, testData.windSpeed);
    EXPECT_DOUBLE_EQ(retrieved->windDirection, testData.windDirection);
}

TEST_F(WeatherStorageTest, HasRecentData) {
    double lat = 40.7128;
    double lon = -74.0060;
    WeatherData testData = createTestData();

    // Store data
    EXPECT_TRUE(storage.storeWeatherData(lat, lon, testData));

    // Check recent data exists
    EXPECT_TRUE(storage.hasRecentData(lat, lon, 60)); // Within 60 minutes

    // Check with different location
    EXPECT_FALSE(storage.hasRecentData(lat + 1.0, lon + 1.0, 60));
}

TEST_F(WeatherStorageTest, GetNearestData) {
    double baseLat = 40.7128;
    double baseLon = -74.0060;
    
    // Store data at different locations
    WeatherData data1 = createTestData(20.0, 5.0);
    WeatherData data2 = createTestData(22.0, 6.0);
    WeatherData data3 = createTestData(18.0, 4.0);

    storage.storeWeatherData(baseLat, baseLon, data1);
    storage.storeWeatherData(baseLat + 0.1, baseLon + 0.1, data2);  // ~15km away
    storage.storeWeatherData(baseLat + 0.5, baseLon + 0.5, data3);  // ~75km away

    // Test finding nearest data within 20km
    auto nearest = storage.getNearestWeatherData(baseLat + 0.05, baseLon + 0.05, 20.0);
    ASSERT_TRUE(nearest.has_value());
    EXPECT_DOUBLE_EQ(nearest->temperature, data2.temperature);

    // Test with location too far from any data
    auto tooFar = storage.getNearestWeatherData(baseLat + 1.0, baseLon + 1.0, 20.0);
    EXPECT_FALSE(tooFar.has_value());
}

TEST_F(WeatherStorageTest, TypicalWeatherStorage) {
    double lat = 40.7128;
    double lon = -74.0060;
    WeatherData testData = createTestData();

    // Store typical weather for current month
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    int currentMonth = ltm->tm_mon + 1;

    EXPECT_TRUE(storage.storeTypicalWeather(lat, lon, currentMonth, testData));

    // Retrieve typical weather
    auto typical = storage.getTypicalWeather(lat, lon);
    ASSERT_TRUE(typical.has_value());
    EXPECT_DOUBLE_EQ(typical->temperature, testData.temperature);
}

TEST_F(WeatherStorageTest, HistoricalStats) {
    double lat = 40.7128;
    double lon = -74.0060;

    // Store multiple data points
    WeatherData data1 = createTestData(20.0, 5.0);
    WeatherData data2 = createTestData(22.0, 6.0);
    WeatherData data3 = createTestData(18.0, 4.0);

    storage.storeWeatherData(lat, lon, data1);
    storage.storeWeatherData(lat, lon, data2);
    storage.storeWeatherData(lat, lon, data3);

    // Get current month
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    int currentMonth = ltm->tm_mon + 1;

    // Get stats
    auto stats = storage.getHistoricalStats(lat, lon, currentMonth);
    ASSERT_TRUE(stats.has_value());

    // Verify averages
    EXPECT_DOUBLE_EQ(stats->avgTemperature, 20.0);  // (20 + 22 + 18) / 3
    EXPECT_DOUBLE_EQ(stats->avgWindSpeed, 5.0);     // (5 + 6 + 4) / 3
}

TEST_F(WeatherStorageTest, ClearOldData) {
    double lat = 40.7128;
    double lon = -74.0060;

    // Store data with old timestamp
    WeatherData oldData = createTestData();
    oldData.timestamp = std::time(nullptr) - 86400;  // 24 hours ago
    storage.storeWeatherData(lat, lon, oldData);

    // Store recent data
    WeatherData newData = createTestData();
    storage.storeWeatherData(lat, lon, newData);

    // Clear data older than 12 hours
    storage.clearOldData(std::time(nullptr) - 43200);

    // Verify old data is gone but new data remains
    auto data = storage.getWeatherData(lat, lon);
    ASSERT_TRUE(data.has_value());
    EXPECT_GT(data->timestamp, std::time(nullptr) - 43200);
}

TEST_F(WeatherStorageTest, WindDirectionDistribution) {
    double lat = 40.7128;
    double lon = -74.0060;

    // Store data with different wind directions
    for (int i = 0; i < 360; i += 45) {
        WeatherData data = createTestData();
        data.windDirection = i;
        storage.storeWeatherData(lat, lon, data);
    }

    // Get current month
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    int currentMonth = ltm->tm_mon + 1;

    // Get stats
    auto stats = storage.getHistoricalStats(lat, lon, currentMonth);
    ASSERT_TRUE(stats.has_value());

    // Verify wind direction frequency
    ASSERT_EQ(stats->windDirectionFrequency.size(), 36);
    
    // Each 45-degree increment should have one entry
    for (int i = 0; i < 36; i++) {
        if (i % 5 == 0) {  // Every 45 degrees (45/10 = 4.5 rounded to 5)
            EXPECT_GT(stats->windDirectionFrequency[i], 0.0);
        }
    }
}
