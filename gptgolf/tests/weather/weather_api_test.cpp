#include <gtest/gtest.h>
#include "weather/weather_api.h"
#include <gmock/gmock.h>  // Include Google Mock
#include <filesystem>
#include <ctime>

class WeatherAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize storage with test database
        dbPath = "test_weather.db";
        storage.initialize(dbPath);
        api = std::make_unique<WeatherAPI>(storage);
        api->initialize("test_api_key");
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
    std::unique_ptr<WeatherAPI> api;
    const double testLat = 40.7128;
    const double testLon = -74.0060;
};

TEST_F(WeatherAPITest, OfflineModeWithNoData) {
    api->setOfflineMode(true);
    WeatherData data;
    
    // With no stored data, getCurrentWeather should fail
    EXPECT_FALSE(api->getCurrentWeather(testLat, testLon, data));
}

TEST_F(WeatherAPITest, OfflineModeWithStoredData) {
    // Store some test data
    WeatherData testData = createTestData();
    storage.storeWeatherData(testLat, testLon, testData);

    // Enable offline mode and try to get data
    api->setOfflineMode(true);
    WeatherData retrievedData;
    EXPECT_TRUE(api->getCurrentWeather(testLat, testLon, retrievedData));

    // Verify retrieved data matches stored data
    EXPECT_DOUBLE_EQ(retrievedData.temperature, testData.temperature);
    EXPECT_DOUBLE_EQ(retrievedData.windSpeed, testData.windSpeed);
}

TEST_F(WeatherAPITest, OfflineModeWithNearbyData) {
    // Store data at a nearby location
    WeatherData testData = createTestData();
    storage.storeWeatherData(testLat + 0.05, testLon + 0.05, testData);  // ~7km away

    // Enable offline mode and try to get data for original location
    api->setOfflineMode(true);
    WeatherData retrievedData;
    EXPECT_TRUE(api->getCurrentWeather(testLat, testLon, retrievedData));

    // Verify we got the nearby data
    EXPECT_DOUBLE_EQ(retrievedData.temperature, testData.temperature);
}

TEST_F(WeatherAPITest, OfflineModeWithTypicalWeather) {
    // Store typical weather data
    WeatherData typicalData = createTestData();
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    int currentMonth = ltm->tm_mon + 1;
    
    storage.storeTypicalWeather(testLat, testLon, currentMonth, typicalData);

    // Enable offline mode and try to get data
    api->setOfflineMode(true);
    WeatherData retrievedData;
    EXPECT_TRUE(api->getCurrentWeather(testLat, testLon, retrievedData));

    // Verify we got the typical weather data
    EXPECT_DOUBLE_EQ(retrievedData.temperature, typicalData.temperature);
}

TEST_F(WeatherAPITest, AutomaticFallbackToOffline) {
    // Store some test data
    WeatherData testData = createTestData();
    storage.storeWeatherData(testLat, testLon, testData);

    // Try to get data with invalid API key (should fail and fall back to stored data)
    api->initialize("invalid_key", false);
    WeatherData retrievedData;
    EXPECT_TRUE(api->getCurrentWeather(testLat, testLon, retrievedData));

    // Verify we got the stored data
    EXPECT_DOUBLE_EQ(retrievedData.temperature, testData.temperature);
}

TEST_F(WeatherAPITest, DataStorageAfterAPIFetch) {
    // Initialize with valid API key (mock would be better here)
    api->initialize("valid_key", false);
    
    // Attempt to get weather data
    WeatherData data;
    if (api->getCurrentWeather(testLat, testLon, data)) {
        // Verify data was stored
        auto storedData = storage.getWeatherData(testLat, testLon);
        ASSERT_TRUE(storedData.has_value());
        EXPECT_DOUBLE_EQ(storedData->temperature, data.temperature);
    }
}

TEST_F(WeatherAPITest, ErrorCallbackTest) {
    bool errorCalled = false;
    std::string errorMessage;

    api->setErrorCallback([&](const std::string& msg) {
        errorCalled = true;
        errorMessage = msg;
    });

    // Try to get data without initialization
    api->initialize("", true);  // Empty API key
    WeatherData data;
    EXPECT_FALSE(api->getCurrentWeather(testLat, testLon, data));
    EXPECT_TRUE(errorCalled);
    EXPECT_FALSE(errorMessage.empty());
}

TEST_F(WeatherAPITest, ModeTransitionTest) {
    // Start in online mode
    api->setOfflineMode(false);
    EXPECT_FALSE(api->isOfflineMode());

    // Switch to offline mode
    api->setOfflineMode(true);
    EXPECT_TRUE(api->isOfflineMode());

    // Store some data
    WeatherData testData = createTestData();
    storage.storeWeatherData(testLat, testLon, testData);

    // Verify we can get data in offline mode
    WeatherData retrievedData;
    EXPECT_TRUE(api->getCurrentWeather(testLat, testLon, retrievedData));
    EXPECT_DOUBLE_EQ(retrievedData.temperature, testData.temperature);

    // Switch back to online mode
    api->setOfflineMode(false);
    EXPECT_FALSE(api->isOfflineMode());
}
