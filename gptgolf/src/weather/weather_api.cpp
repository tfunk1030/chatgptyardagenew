#include "weather/weather_api.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

// Callback function to write API response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class WeatherAPI::Impl {
public:
    Impl() : curl(nullptr) {
        curl = curl_easy_init();
    }

    ~Impl() {
        if (curl) curl_easy_cleanup(curl);
    }

    bool fetch(const std::string& apiKey, double lat, double lon, std::string& response) {
        if (!curl) return false;

        std::stringstream url;
        url << "https://api.tomorrow.io/v4/weather/realtime"
            << "?location=" << lat << "," << lon
            << "&apikey=" << apiKey
            << "&units=metric";

        curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        return (res == CURLE_OK);
    }

private:
    CURL* curl;
};

WeatherAPI::WeatherAPI(WeatherStorage& weatherStorage)
    : pImpl(new Impl())
    , storage(weatherStorage)
    , initialized(false)
    , offlineMode(false) {}

WeatherAPI::~WeatherAPI() = default;

bool WeatherAPI::initialize(const std::string& key, bool useOfflineMode) {
    apiKey = key;
    offlineMode = useOfflineMode;
    initialized = true;
    return true;
}

void WeatherAPI::setOfflineMode(bool enabled) {
    offlineMode = enabled;
}

bool WeatherAPI::isOfflineMode() const {
    return offlineMode;
}

bool WeatherAPI::getCurrentWeather(double latitude, double longitude, WeatherData& data) {
    if (!initialized) {
        if (errorCallback) errorCallback("API not initialized");
        return false;
    }

    // Try to get data from storage first
    if (storage.hasRecentData(latitude, longitude, MAX_CACHE_AGE_MINUTES)) {
        auto storedData = storage.getWeatherData(latitude, longitude);
        if (storedData) {
            data = *storedData;
            return true;
        }
    }

    // If in offline mode, try to get nearest data or typical weather
    if (offlineMode) {
        return getOfflineWeather(latitude, longitude, data);
    }

    // Fetch new data from API
    if (!fetchFromAPI(latitude, longitude, data)) {
        // If API fetch fails, fall back to offline data
        if (errorCallback) errorCallback("API request failed, falling back to offline data");
        return getOfflineWeather(latitude, longitude, data);
    }

    // Store the new data
    storage.storeWeatherData(latitude, longitude, data);
    return true;
}

bool WeatherAPI::getOfflineWeather(double latitude, double longitude, WeatherData& data) {
    // Try to get nearest recent data
    auto nearestData = storage.getNearestWeatherData(latitude, longitude, MAX_DISTANCE_KM);
    if (nearestData) {
        data = *nearestData;
        return true;
    }

    // Try to get typical weather for the location
    auto typicalData = storage.getTypicalWeather(latitude, longitude);
    if (typicalData) {
        data = *typicalData;
        return true;
    }

    if (errorCallback) errorCallback("No offline weather data available");
    return false;
}

bool WeatherAPI::fetchFromAPI(double latitude, double longitude, WeatherData& data) {
    std::string response;
    if (!pImpl->fetch(apiKey, latitude, longitude, response)) {
        return false;
    }

    try {
        if (!parseWeatherResponse(response, data)) {
            return false;
        }
        return true;
    }
    catch (const std::exception& e) {
        if (errorCallback) errorCallback(std::string("Error processing weather data: ") + e.what());
        return false;
    }
}

bool WeatherAPI::parseWeatherResponse(const std::string& response, WeatherData& data) {
    try {
        json j = json::parse(response);
        auto& values = j["data"]["values"];

        data.temperature = values["temperature"].get<double>();
        data.humidity = values["humidity"].get<double>();
        data.pressure = values["pressureSeaLevel"].get<double>();
        data.windSpeed = values["windSpeed"].get<double>();
        data.windDirection = values["windDirection"].get<double>();
        data.precipitation = values["precipitationIntensity"].get<double>();
        data.timestamp = std::time(nullptr);

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

void WeatherAPI::setErrorCallback(std::function<void(const std::string&)> callback) {
    errorCallback = callback;
}

bool WeatherAPI::isInitialized() const {
    return initialized;
}
