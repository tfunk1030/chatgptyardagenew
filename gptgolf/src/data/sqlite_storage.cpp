#include "../../include/data/sqlite_storage.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <sstream>

namespace gptgolf {
namespace data {

using json = nlohmann::json;

// SQL statements for table creation
const char* SQLiteStorage::SHOTS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS shots (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        initial_velocity REAL NOT NULL,
        spin_rate REAL NOT NULL,
        launch_angle REAL NOT NULL,
        weather_data TEXT NOT NULL,
        club_used TEXT NOT NULL,
        actual_distance REAL NOT NULL,
        predicted_distance REAL NOT NULL,
        lateral_deviation REAL NOT NULL,
        timestamp INTEGER NOT NULL
    )
)";

const char* SQLiteStorage::CLUBS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS clubs (
        name TEXT PRIMARY KEY,
        avg_distance REAL NOT NULL,
        avg_spin_rate REAL NOT NULL,
        avg_launch_angle REAL NOT NULL,
        total_shots INTEGER NOT NULL,
        last_updated INTEGER NOT NULL,
        distance_deviation REAL NOT NULL,
        direction_deviation REAL NOT NULL
    )
)";

const char* SQLiteStorage::PREFS_TABLE = R"(
    CREATE TABLE IF NOT EXISTS preferences (
        key TEXT PRIMARY KEY,
        value TEXT NOT NULL
    )
)";

SQLiteStorage::SQLiteStorage(const std::string& dbPath) : db_(nullptr) {
    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc) {
        std::string error = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw std::runtime_error("Cannot open database: " + error);
    }
    initializeTables();
}

SQLiteStorage::~SQLiteStorage() {
    if (db_) {
        sqlite3_close(db_);
    }
}

void SQLiteStorage::initializeTables() {
    executeStatement(SHOTS_TABLE);
    executeStatement(CLUBS_TABLE);
    executeStatement(PREFS_TABLE);
}

void SQLiteStorage::executeStatement(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL error: " + error);
    }
}

bool SQLiteStorage::saveShotData(const ShotData& shot) {
    const char* sql = R"(
        INSERT INTO shots (
            initial_velocity, spin_rate, launch_angle, weather_data,
            club_used, actual_distance, predicted_distance,
            lateral_deviation, timestamp
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_double(stmt, 1, shot.initialVelocity);
    sqlite3_bind_double(stmt, 2, shot.spinRate);
    sqlite3_bind_double(stmt, 3, shot.launchAngle);
    sqlite3_bind_text(stmt, 4, weatherDataToJson(shot.conditions).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, shot.clubUsed.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 6, shot.actualDistance);
    sqlite3_bind_double(stmt, 7, shot.predictedDistance);
    sqlite3_bind_double(stmt, 8, shot.lateralDeviation);
    sqlite3_bind_int64(stmt, 9, shot.timestamp);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::vector<ShotData> SQLiteStorage::getShotHistory(size_t limit) {
    std::vector<ShotData> shots;
    const char* sql = R"(
        SELECT * FROM shots ORDER BY timestamp DESC LIMIT ?
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return shots;
    }

    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(limit));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ShotData shot;
        shot.initialVelocity = sqlite3_column_double(stmt, 1);
        shot.spinRate = sqlite3_column_double(stmt, 2);
        shot.launchAngle = sqlite3_column_double(stmt, 3);
        shot.conditions = jsonToWeatherData(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        shot.clubUsed = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        shot.actualDistance = sqlite3_column_double(stmt, 6);
        shot.predictedDistance = sqlite3_column_double(stmt, 7);
        shot.lateralDeviation = sqlite3_column_double(stmt, 8);
        shot.timestamp = sqlite3_column_int64(stmt, 9);
        shots.push_back(shot);
    }

    sqlite3_finalize(stmt);
    return shots;
}

std::vector<ShotData> SQLiteStorage::getShotsByClub(const std::string& clubName) {
    std::vector<ShotData> shots;
    const char* sql = R"(
        SELECT * FROM shots WHERE club_used = ? ORDER BY timestamp DESC
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return shots;
    }

    sqlite3_bind_text(stmt, 1, clubName.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ShotData shot;
        shot.initialVelocity = sqlite3_column_double(stmt, 1);
        shot.spinRate = sqlite3_column_double(stmt, 2);
        shot.launchAngle = sqlite3_column_double(stmt, 3);
        shot.conditions = jsonToWeatherData(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        shot.clubUsed = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        shot.actualDistance = sqlite3_column_double(stmt, 6);
        shot.predictedDistance = sqlite3_column_double(stmt, 7);
        shot.lateralDeviation = sqlite3_column_double(stmt, 8);
        shot.timestamp = sqlite3_column_int64(stmt, 9);
        shots.push_back(shot);
    }

    sqlite3_finalize(stmt);
    return shots;
}

bool SQLiteStorage::saveClubProfile(const ClubProfile& club) {
    const char* sql = R"(
        INSERT INTO clubs (
            name, avg_distance, avg_spin_rate, avg_launch_angle,
            total_shots, last_updated, distance_deviation, direction_deviation
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, club.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, club.avgDistance);
    sqlite3_bind_double(stmt, 3, club.avgSpinRate);
    sqlite3_bind_double(stmt, 4, club.avgLaunchAngle);
    sqlite3_bind_int64(stmt, 5, club.totalShots);
    sqlite3_bind_int64(stmt, 6, club.lastUpdated);
    sqlite3_bind_double(stmt, 7, club.distanceDeviation);
    sqlite3_bind_double(stmt, 8, club.directionDeviation);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool SQLiteStorage::updateClubProfile(const ClubProfile& club) {
    const char* sql = R"(
        UPDATE clubs SET
            avg_distance = ?,
            avg_spin_rate = ?,
            avg_launch_angle = ?,
            total_shots = ?,
            last_updated = ?,
            distance_deviation = ?,
            direction_deviation = ?
        WHERE name = ?
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_double(stmt, 1, club.avgDistance);
    sqlite3_bind_double(stmt, 2, club.avgSpinRate);
    sqlite3_bind_double(stmt, 3, club.avgLaunchAngle);
    sqlite3_bind_int64(stmt, 4, club.totalShots);
    sqlite3_bind_int64(stmt, 5, club.lastUpdated);
    sqlite3_bind_double(stmt, 6, club.distanceDeviation);
    sqlite3_bind_double(stmt, 7, club.directionDeviation);
    sqlite3_bind_text(stmt, 8, club.name.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::optional<ClubProfile> SQLiteStorage::getClubProfile(const std::string& name) {
    const char* sql = "SELECT * FROM clubs WHERE name = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        ClubProfile club;
        club.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        club.avgDistance = sqlite3_column_double(stmt, 1);
        club.avgSpinRate = sqlite3_column_double(stmt, 2);
        club.avgLaunchAngle = sqlite3_column_double(stmt, 3);
        club.totalShots = sqlite3_column_int64(stmt, 4);
        club.lastUpdated = sqlite3_column_int64(stmt, 5);
        club.distanceDeviation = sqlite3_column_double(stmt, 6);
        club.directionDeviation = sqlite3_column_double(stmt, 7);

        sqlite3_finalize(stmt);
        return club;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<ClubProfile> SQLiteStorage::getAllClubProfiles() {
    std::vector<ClubProfile> clubs;
    const char* sql = "SELECT * FROM clubs ORDER BY name";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return clubs;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ClubProfile club;
        club.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        club.avgDistance = sqlite3_column_double(stmt, 1);
        club.avgSpinRate = sqlite3_column_double(stmt, 2);
        club.avgLaunchAngle = sqlite3_column_double(stmt, 3);
        club.totalShots = sqlite3_column_int64(stmt, 4);
        club.lastUpdated = sqlite3_column_int64(stmt, 5);
        club.distanceDeviation = sqlite3_column_double(stmt, 6);
        club.directionDeviation = sqlite3_column_double(stmt, 7);
        clubs.push_back(club);
    }

    sqlite3_finalize(stmt);
    return clubs;
}

bool SQLiteStorage::savePreference(const std::string& key, const std::string& value) {
    const char* sql = R"(
        INSERT OR REPLACE INTO preferences (key, value) VALUES (?, ?)
    )";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::string SQLiteStorage::getPreference(const std::string& key, const std::string& defaultValue) {
    const char* sql = "SELECT value FROM preferences WHERE key = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return defaultValue;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        return value;
    }

    sqlite3_finalize(stmt);
    return defaultValue;
}

std::string SQLiteStorage::weatherDataToJson(const weather::WeatherData& data) {
    json j;
    j["temperature"] = data.temperature;
    j["humidity"] = data.humidity;
    j["pressure"] = data.pressure;
    j["windSpeed"] = data.windSpeed;
    j["windDirection"] = data.windDirection;
    return j.dump();
}

weather::WeatherData SQLiteStorage::jsonToWeatherData(const std::string& jsonStr) {
    weather::WeatherData data;
    json j = json::parse(jsonStr);
    data.temperature = j["temperature"];
    data.humidity = j["humidity"];
    data.pressure = j["pressure"];
    data.windSpeed = j["windSpeed"];
    data.windDirection = j["windDirection"];
    return data;
}

} // namespace data
} // namespace gptgolf
