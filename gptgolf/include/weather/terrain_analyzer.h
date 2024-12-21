#pragma once

#include "physics/wind.h"
#include "weather/weather_data.h"
#include "weather/weather_storage.h"
#include <string>
#include <vector>
#include <optional>
#include <ctime>

// Land use classification based on OpenStreetMap data
enum class LandUseType {
    WATER,              // Ocean, lakes, rivers
    COASTAL,            // Coastal areas, beaches
    GRASSLAND,          // Open fields, golf courses
    FOREST,             // Dense vegetation
    SUBURBAN,           // Residential areas
    URBAN,              // City centers
    INDUSTRIAL,         // Industrial zones
    MOUNTAIN,           // High elevation, complex terrain
    UNKNOWN             // Default when data unavailable
};

// Terrain analysis result
struct TerrainAnalysis {
    TerrainParameters params;
    LandUseType landUse;
    double elevation;           // meters above sea level
    double roughnessVariation; // local terrain roughness variation
    bool isComplex;           // true for terrain with significant elevation changes
    
    TerrainAnalysis()
        : landUse(LandUseType::UNKNOWN)
        , elevation(0.0)
        , roughnessVariation(0.0)
        , isComplex(false) {}
};

// Wind pattern data for statistical analysis
struct WindPattern {
    double speed;
    double direction;
    double gustSpeed;
    double temperature;
    double pressure;
    std::time_t timestamp;
    
    WindPattern()
        : speed(0.0)
        , direction(0.0)
        , gustSpeed(0.0)
        , temperature(0.0)
        , pressure(0.0)
        , timestamp(0) {}
};

// Statistical wind analysis
struct WindStatistics {
    double meanSpeed;
    double maxSpeed;
    double speedVariation;
    double prevailingDirection;
    double directionVariation;
    double gustFactor;         // ratio of gust speed to mean speed
    double turbulenceIntensity;
    
    WindStatistics()
        : meanSpeed(0.0)
        , maxSpeed(0.0)
        , speedVariation(0.0)
        , prevailingDirection(0.0)
        , directionVariation(0.0)
        , gustFactor(0.0)
        , turbulenceIntensity(0.0) {}
};

class TerrainAnalyzer {
public:
    TerrainAnalyzer(WeatherStorage& storage);
    
    // Analyze terrain at a location
    TerrainAnalysis analyzeTerrain(double latitude, double longitude);
    
    // Get recommended wind profile for conditions
    WindProfile recommendProfile(const TerrainAnalysis& terrain,
                               const WeatherData& weather);
    
    // Store and analyze wind patterns
    void storeWindPattern(double latitude, double longitude,
                         const WeatherData& weather);
    
    // Get wind statistics for location
    std::optional<WindStatistics> getWindStats(double latitude, double longitude,
                                             std::time_t startTime,
                                             std::time_t endTime);
    
    // Get typical wind conditions by time of day
    std::vector<WindPattern> getTypicalPatterns(double latitude, double longitude,
                                              int hourOfDay);

private:
    WeatherStorage& storage;
    
    // Internal analysis methods
    LandUseType detectLandUse(double latitude, double longitude);
    double getElevation(double latitude, double longitude);
    bool isComplexTerrain(double latitude, double longitude);
    TerrainParameters deriveParameters(const TerrainAnalysis& analysis);
    
    // Wind pattern analysis
    WindStatistics calculateStats(const std::vector<WindPattern>& patterns);
    double calculateTurbulenceIntensity(const std::vector<WindPattern>& patterns);
    
    // Constants
    static constexpr double COMPLEX_TERRAIN_THRESHOLD = 100.0;  // meters elevation change
    static constexpr double MIN_PATTERNS_FOR_STATS = 10;        // minimum patterns for valid statistics
    static constexpr int HOURS_IN_DAY = 24;
    static constexpr double GUST_FACTOR_THRESHOLD = 1.5;        // threshold for considering gusty conditions
};
