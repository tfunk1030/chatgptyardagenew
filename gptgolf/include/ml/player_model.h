#pragma once

#include <string>
#include <map>
#include <vector>
#include "prediction_model.h"
#include "../data/storage.h"

/**
 * @file player_model.h
 * @brief Player-specific shot prediction and analysis
 *
 * This module extends the base prediction model with player-specific
 * analysis and adjustments. It tracks individual player tendencies,
 * skill progression, and adapts predictions based on personal patterns
 * and preferences.
 */

namespace gptgolf {
namespace ml {

/**
 * @brief Represents a specific player tendency or pattern
 *
 * Captures recurring patterns in a player's shots, such as
 * consistent slice or hook tendencies, along with the conditions
 * under which they occur.
 */
struct PlayerTendency {
    std::string pattern;      //!< Type of tendency (e.g., "slice", "hook", "push", "pull")
    double magnitude;         //!< Strength of the tendency (0-1)
    double consistency;       //!< How consistently the tendency occurs (0-1)
    std::vector<std::string> conditions; //!< Conditions triggering the tendency

    /**
     * @brief Check if tendency is significant
     * @return true if magnitude >= 0.3 and consistency >= 0.5
     */
    bool isSignificant() const {
        return magnitude >= 0.3 && consistency >= 0.5;
    }
};

/**
 * @brief Comprehensive player performance profile
 *
 * Contains all analyzed characteristics and tendencies for a specific
 * player, including club-specific patterns and environmental effects.
 */
struct PlayerProfile {
    std::string playerId;     //!< Unique player identifier
    std::map<std::string, PlayerTendency> clubTendencies;  //!< Tendencies for each club
    std::map<std::string, double> conditionFactors;        //!< Weather impact factors
    double skillLevel;        //!< Overall skill rating (0-1)
    size_t totalShots;       //!< Total shots recorded
    std::time_t lastUpdated; //!< Last profile update timestamp

    /**
     * @brief Check if profile has sufficient data
     * @return true if totalShots >= 50
     */
    bool hasReliableData() const {
        return totalShots >= 50;
    }
};

/**
 * @brief Player-specific shot prediction model
 *
 * Extends the base prediction model with player-specific adjustments
 * and analysis. Tracks and learns from individual player patterns
 * to provide more accurate, personalized predictions.
 */
class PlayerModel : public PredictionModel {
public:
    /**
     * @brief Construct a new Player Model
     * @param storage Reference to shot data storage
     * @param collector Reference to data collection system
     */
    PlayerModel(data::IStorage& storage, DataCollector& collector);

    /**
     * @brief Predict shot outcome with player-specific adjustments
     *
     * Overrides base prediction to include individual player tendencies
     * and patterns in the prediction calculations.
     *
     * @param clubName Club to be used
     * @param conditions Current weather conditions
     * @param swingSpeed Optional known swing speed
     * @return PredictionResult with player-adjusted predictions
     */
    PredictionResult predictShot(
        const std::string& clubName,
        const weather::WeatherData& conditions,
        double swingSpeed = 0.0
    ) override;

    /** @name Player Profile Management
     * Methods for managing and analyzing player data
     * @{
     */
    /**
     * @brief Update player profile with new shot data
     *
     * Processes new shot data to update player tendencies, skill level,
     * and condition factors.
     *
     * @param playerId Player identifier
     * @param shot New shot data
     */
    void updatePlayerProfile(const std::string& playerId, const data::ShotData& shot);

    /**
     * @brief Retrieve player's current profile
     * @param playerId Player identifier
     * @return PlayerProfile containing current analysis
     */
    PlayerProfile getPlayerProfile(const std::string& playerId);

    /**
     * @brief Analyze player's shot patterns
     *
     * Performs deep analysis of player's shot history to identify
     * recurring patterns and tendencies.
     *
     * @param playerId Player identifier
     * @return Vector of identified tendencies
     */
    std::vector<PlayerTendency> analyzePlayerTendencies(const std::string& playerId);
    /** @} */

    /**
     * @brief Train model including player-specific data
     *
     * Extends base training to incorporate player-specific patterns
     * and adjustments.
     *
     * @param trainingData Vector of historical shot data
     */
    void train(const std::vector<data::ShotData>& trainingData) override;

protected:
    /** @name Analysis Methods
     * Helper methods for player analysis
     * @{
     */
    /**
     * @brief Calculate player-specific shot adjustments
     *
     * @param playerId Player identifier
     * @param clubName Club being used
     * @param conditions Current weather conditions
     * @return Adjustment factor for predictions
     */
    double calculatePlayerAdjustment(
        const std::string& playerId,
        const std::string& clubName,
        const weather::WeatherData& conditions
    );

    /**
     * @brief Analyze tendency for specific club
     *
     * @param shots Historical shots with club
     * @param clubName Club being analyzed
     * @return Identified tendency pattern
     */
    PlayerTendency analyzeTendency(
        const std::vector<data::ShotData>& shots,
        const std::string& clubName
    );

    /**
     * @brief Update player's skill level
     *
     * @param profile Player profile to update
     * @param shot New shot data
     */
    void updateSkillLevel(PlayerProfile& profile, const data::ShotData& shot);
    /** @} */

private:
    std::map<std::string, PlayerProfile> playerProfiles_; //!< Cache of player profiles
    double playerFactorWeight_ = 0.3;  //!< Weight for player-specific adjustments

    /**
     * @brief Calculate consistency score
     * @param shots Vector of shots to analyze
     * @return Consistency score (0-1)
     */
    double calculateConsistency(const std::vector<data::ShotData>& shots) const;
};

} // namespace ml
} // namespace gptgolf
