#include "../../include/ml/player_model.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace gptgolf {
namespace ml {

PlayerModel::PlayerModel(data::IStorage& storage, DataCollector& collector)
    : PredictionModel(storage, collector) {}

PredictionResult PlayerModel::predictShot(
    const std::string& clubName,
    const weather::WeatherData& conditions,
    double swingSpeed
) {
    // Get base prediction from parent class
    auto basePrediction = PredictionModel::predictShot(clubName, conditions, swingSpeed);

    // Get current player ID from storage preferences
    std::string playerId = storage_.getPreference("current_player_id");
    if (playerId.empty()) {
        return basePrediction; // No player-specific adjustments
    }

    // Apply player-specific adjustments
    double playerAdjustment = calculatePlayerAdjustment(playerId, clubName, conditions);
    
    // Blend base prediction with player adjustment
    basePrediction.predictedDistance *= (1.0 + playerAdjustment * playerFactorWeight_);
    
    // Adjust confidence based on player profile
    auto profile = getPlayerProfile(playerId);
    if (profile.totalShots > 0) {
        // Increase confidence if player has consistent performance with this club
        if (profile.clubTendencies.count(clubName) > 0) {
            auto& tendency = profile.clubTendencies[clubName];
            basePrediction.confidence *= (0.5 + 0.5 * tendency.consistency);
        }
    }

    // Add player-specific factors to the prediction
    if (profile.clubTendencies.count(clubName) > 0) {
        auto& tendency = profile.clubTendencies[clubName];
        basePrediction.factors.push_back(
            "Player tendency: " + tendency.pattern + 
            " (consistency: " + std::to_string(int(tendency.consistency * 100)) + "%)"
        );
    }

    return basePrediction;
}

void PlayerModel::updatePlayerProfile(
    const std::string& playerId,
    const data::ShotData& shot
) {
    auto& profile = playerProfiles_[playerId];
    profile.playerId = playerId;
    profile.totalShots++;
    profile.lastUpdated = std::time(nullptr);

    // Analyze the shot and update tendencies
    auto pattern = collector_.processShotData(shot);
    
    // Update club-specific tendency
    auto& tendency = profile.clubTendencies[shot.clubUsed];
    tendency.pattern = pattern.pattern_type;
    tendency.magnitude = std::abs(pattern.lateralError) / 50.0; // Normalize to 0-1
    
    // Update consistency calculation
    auto recentShots = storage_.getShotsByClub(shot.clubUsed);
    tendency.consistency = 1.0 - std::min(1.0, 
        pattern.distanceError / (shot.predictedDistance * 0.1)  // 10% error = 0 consistency
    );

    // Update condition factors
    profile.conditionFactors["wind"] = std::abs(shot.conditions.windSpeed * 0.1);
    profile.conditionFactors["temperature"] = std::abs(shot.conditions.temperature - 20.0) * 0.05;
    profile.conditionFactors["humidity"] = shot.conditions.humidity * 0.01;

    // Update overall skill level
    updateSkillLevel(profile, shot);
}

PlayerProfile PlayerModel::getPlayerProfile(const std::string& playerId) {
    if (playerProfiles_.count(playerId) == 0) {
        // Initialize new profile
        PlayerProfile profile;
        profile.playerId = playerId;
        profile.skillLevel = 0.5; // Start at middle skill level
        profile.totalShots = 0;
        profile.lastUpdated = std::time(nullptr);
        return profile;
    }
    return playerProfiles_[playerId];
}

std::vector<PlayerTendency> PlayerModel::analyzePlayerTendencies(
    const std::string& playerId
) {
    std::vector<PlayerTendency> tendencies;
    auto profile = getPlayerProfile(playerId);

    for (const auto& [club, tendency] : profile.clubTendencies) {
        if (tendency.consistency > 0.3) { // Only include significant tendencies
            tendencies.push_back(tendency);
        }
    }

    // Sort by consistency
    std::sort(tendencies.begin(), tendencies.end(),
        [](const PlayerTendency& a, const PlayerTendency& b) {
            return a.consistency > b.consistency;
        });

    return tendencies;
}

void PlayerModel::train(const std::vector<data::ShotData>& trainingData) {
    // First, train the base model
    PredictionModel::train(trainingData);

    // Group shots by player
    std::map<std::string, std::vector<data::ShotData>> playerShots;
    for (const auto& shot : trainingData) {
        std::string playerId = shot.clubUsed.substr(0, shot.clubUsed.find('_'));
        playerShots[playerId].push_back(shot);
    }

    // Train player-specific adjustments
    for (const auto& [playerId, shots] : playerShots) {
        for (const auto& shot : shots) {
            updatePlayerProfile(playerId, shot);
        }
    }
}

double PlayerModel::calculatePlayerAdjustment(
    const std::string& playerId,
    const std::string& clubName,
    const weather::WeatherData& conditions
) {
    auto profile = getPlayerProfile(playerId);
    double adjustment = 0.0;

    // Apply club-specific tendency
    if (profile.clubTendencies.count(clubName) > 0) {
        auto& tendency = profile.clubTendencies[clubName];
        adjustment += tendency.magnitude * tendency.consistency;
    }

    // Apply weather condition factors
    for (const auto& [condition, factor] : profile.conditionFactors) {
        if (condition == "wind" && conditions.windSpeed > 10.0) {
            adjustment += factor * (conditions.windSpeed / 20.0);
        }
        else if (condition == "temperature" && std::abs(conditions.temperature - 20.0) > 10.0) {
            adjustment += factor;
        }
        else if (condition == "humidity" && conditions.humidity > 70.0) {
            adjustment += factor;
        }
    }

    // Scale adjustment based on skill level
    adjustment *= profile.skillLevel;

    return std::clamp(adjustment, -0.5, 0.5); // Limit adjustment to ±50%
}

PlayerTendency PlayerModel::analyzeTendency(
    const std::vector<data::ShotData>& shots,
    [[maybe_unused]] const std::string& clubName
) {
    PlayerTendency tendency;
    if (shots.empty()) return tendency;

    // Calculate average lateral deviation
    double avgLateral = 0.0;
    double avgDistance = 0.0;
    std::vector<double> lateralDeviations;
    std::vector<double> distanceDeviations;

    for (const auto& shot : shots) {
        avgLateral += shot.lateralDeviation;
        avgDistance += shot.actualDistance - shot.predictedDistance;
        lateralDeviations.push_back(shot.lateralDeviation);
        distanceDeviations.push_back(shot.actualDistance - shot.predictedDistance);
    }
    avgLateral /= shots.size();
    avgDistance /= shots.size();

    // Determine pattern type
    if (std::abs(avgLateral) > 10.0) {
        tendency.pattern = avgLateral > 0 ? "slice" : "hook";
    } else if (std::abs(avgDistance) > 10.0) {
        tendency.pattern = avgDistance > 0 ? "long" : "short";
    } else {
        tendency.pattern = "consistent";
    }

    // Calculate consistency
    double lateralStdDev = std::sqrt(std::accumulate(lateralDeviations.begin(), 
        lateralDeviations.end(), 0.0, 
        [avgLateral](double sum, double x) {
            return sum + (x - avgLateral) * (x - avgLateral);
        }) / shots.size());

    tendency.consistency = 1.0 - std::min(1.0, lateralStdDev / 50.0);
    tendency.magnitude = std::abs(avgLateral) / 50.0;

    return tendency;
}

void PlayerModel::updateSkillLevel(PlayerProfile& profile, const data::ShotData& shot) {
    // Get recent shots for consistency analysis
    auto recentShots = storage_.getShotsByClub(shot.clubUsed);
    if (recentShots.empty()) return;

    // Calculate accuracy metrics
    double distanceError = std::abs(shot.actualDistance - shot.predictedDistance);
    double normalizedError = distanceError / shot.predictedDistance;
    
    // Update skill level using exponential moving average
    double alpha = 0.1; // Learning rate
    double shotSkill = 1.0 - std::min(1.0, normalizedError);
    profile.skillLevel = (1.0 - alpha) * profile.skillLevel + alpha * shotSkill;
}

} // namespace ml
} // namespace gptgolf
