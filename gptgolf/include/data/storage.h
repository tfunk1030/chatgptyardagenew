#pragma once

#include <string>
#include <vector>
#include <optional>
#include <ctime>
#include "../weather/weather_data.h"

namespace gptgolf {
namespace data {

// Forward declarations
struct ShotData;
struct ClubProfile;

/**
 * @brief Interface for data storage operations
 * 
 * Defines the contract for storing and retrieving golf shot data,
 * club profiles, and user preferences.
 */
class IStorage {
public:
    virtual ~IStorage() = default;

    // Shot data operations
    virtual bool saveShotData(const ShotData& shot) = 0;
    virtual std::vector<ShotData> getShotHistory(size_t limit = 100) = 0;
    virtual std::vector<ShotData> getShotsByClub(const std::string& clubName) = 0;

    // Club profile operations
    virtual bool saveClubProfile(const ClubProfile& club) = 0;
    virtual bool updateClubProfile(const ClubProfile& club) = 0;
    virtual std::optional<ClubProfile> getClubProfile(const std::string& name) = 0;
    virtual std::vector<ClubProfile> getAllClubProfiles() = 0;

    // Preference operations
    virtual bool savePreference(const std::string& key, const std::string& value) = 0;
    virtual std::string getPreference(const std::string& key, const std::string& defaultValue = "") = 0;
};

/**
 * @brief Represents a single golf shot with all relevant data
 */
struct ShotData {
    double initialVelocity;     // Initial ball velocity in m/s
    double spinRate;            // Ball spin rate in rpm
    double launchAngle;         // Launch angle in degrees
    weather::WeatherData conditions; // Weather conditions during shot
    std::string clubUsed;       // Name of the club used
    double actualDistance;      // Actual distance achieved in meters
    double predictedDistance;   // Distance predicted by the system
    double lateralDeviation;    // Lateral deviation from target line in meters
    std::time_t timestamp;      // When the shot was taken

    // Constructor with default values
    ShotData() : initialVelocity(0), spinRate(0), launchAngle(0),
                 actualDistance(0), predictedDistance(0), lateralDeviation(0),
                 timestamp(std::time(nullptr)) {}
};

/**
 * @brief Represents a golf club's profile and performance data
 */
struct ClubProfile {
    std::string name;           // Club name/identifier
    double avgDistance;         // Average distance in meters
    double avgSpinRate;         // Average spin rate in rpm
    double avgLaunchAngle;      // Average launch angle in degrees
    size_t totalShots;         // Total number of shots recorded
    std::time_t lastUpdated;    // Last profile update timestamp

    // Performance statistics
    double distanceDeviation;   // Standard deviation in distance
    double directionDeviation;  // Standard deviation in direction
    
    // Constructor with default values
    ClubProfile() : avgDistance(0), avgSpinRate(0), avgLaunchAngle(0),
                    totalShots(0), lastUpdated(std::time(nullptr)),
                    distanceDeviation(0), directionDeviation(0) {}
};

} // namespace data
} // namespace gptgolf
