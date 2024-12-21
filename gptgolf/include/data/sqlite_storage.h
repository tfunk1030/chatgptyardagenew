#pragma once

#include <sqlite3.h>
#include <memory>
#include <string>
#include "storage.h"

namespace gptgolf {
namespace data {

/**
 * @brief SQLite implementation of the storage interface
 * 
 * Provides persistent storage using SQLite database for shot data,
 * club profiles, and user preferences.
 */
class SQLiteStorage : public IStorage {
public:
    /**
     * @brief Construct a new SQLite Storage object
     * 
     * @param dbPath Path to the SQLite database file
     * @throws std::runtime_error if database initialization fails
     */
    explicit SQLiteStorage(const std::string& dbPath);
    ~SQLiteStorage();

    // Shot data operations
    bool saveShotData(const ShotData& shot) override;
    std::vector<ShotData> getShotHistory(size_t limit = 100) override;
    std::vector<ShotData> getShotsByClub(const std::string& clubName) override;

    // Club profile operations
    bool saveClubProfile(const ClubProfile& club) override;
    bool updateClubProfile(const ClubProfile& club) override;
    std::optional<ClubProfile> getClubProfile(const std::string& name) override;
    std::vector<ClubProfile> getAllClubProfiles() override;

    // Preference operations
    bool savePreference(const std::string& key, const std::string& value) override;
    std::string getPreference(const std::string& key, const std::string& defaultValue = "") override;

private:
    /**
     * @brief Initialize database tables
     * 
     * Creates necessary tables if they don't exist
     * @throws std::runtime_error if table creation fails
     */
    void initializeTables();

    /**
     * @brief Execute a SQL statement
     * 
     * @param sql SQL statement to execute
     * @throws std::runtime_error if execution fails
     */
    void executeStatement(const std::string& sql);

    /**
     * @brief Convert WeatherData to/from JSON string
     */
    std::string weatherDataToJson(const weather::WeatherData& data);
    weather::WeatherData jsonToWeatherData(const std::string& json);

    sqlite3* db_;                      // SQLite database handle
    static const char* SHOTS_TABLE;    // SQL for shots table creation
    static const char* CLUBS_TABLE;    // SQL for clubs table creation
    static const char* PREFS_TABLE;    // SQL for preferences table creation
};

} // namespace data
} // namespace gptgolf
