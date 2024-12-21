#include "data/club_analysis.h"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace gptgolf { namespace data {

constexpr double PI = 3.14159265358979323846;

ClubAnalysis::ClubAnalysis(IStorage& storage) : storage_(storage) {}

ClubRecommendation ClubAnalysis::recommendClub(
    double targetDistance,
    const weather::WeatherData& conditions
) {
    ClubRecommendation recommendation;
    double bestConfidence = 0.0;

    auto clubs = storage_.getAllClubProfiles();

    for (const auto& club : clubs) {
        double adjustedDistance = adjustDistanceForConditions(
            club.avgDistance,
            conditions
        );

        double confidence = calculateConfidenceScore(
            targetDistance,
            club,
            conditions
        );

        if (confidence > bestConfidence) {
            bestConfidence = confidence;
            recommendation.clubName = club.name;
            recommendation.confidenceScore = confidence;
            recommendation.expectedDistance = adjustedDistance;
            recommendation.expectedAccuracy = club.directionDeviation;

            std::stringstream reason;
            reason << "Expected carry: " << static_cast<int>(adjustedDistance)
                  << "m with " << static_cast<int>(confidence * 100)
                  << "% confidence. ";
            
            if (club.totalShots > 10) {
                reason << "Based on " << club.totalShots << " recorded shots. ";
            }

            if (std::abs(conditions.windSpeed) > 5.0) {
                reason << "Wind adjustment applied. ";
            }

            recommendation.reasoning = reason.str();
        }
    }

    return recommendation;
}

ClubStatistics ClubAnalysis::analyzeClubPerformance(const std::string& clubName) {
    ClubStatistics stats;
    auto shots = getRecentShots(clubName);

    if (shots.empty()) {
        return stats;
    }

    double sumDistance = 0.0;
    double sumDeviation = 0.0;
    for (const auto& shot : shots) {
        sumDistance += shot.actualDistance;
        sumDeviation += shot.lateralDeviation;
    }

    stats.meanDistance = sumDistance / shots.size();
    stats.meanAccuracy = sumDeviation / shots.size();
    stats.sampleSize = shots.size();

    double sumDistanceSquares = 0.0;
    double sumDeviationSquares = 0.0;
    for (const auto& shot : shots) {
        double distanceDiff = shot.actualDistance - stats.meanDistance;
        double deviationDiff = shot.lateralDeviation - stats.meanAccuracy;
        sumDistanceSquares += distanceDiff * distanceDiff;
        sumDeviationSquares += deviationDiff * deviationDiff;
    }

    stats.distanceStdDev = std::sqrt(sumDistanceSquares / shots.size());
    stats.accuracyStdDev = std::sqrt(sumDeviationSquares / shots.size());

    double maxAllowedDistanceVar = stats.meanDistance * 0.15;
    double maxAllowedAccuracyVar = 20.0;

    double distanceConsistency = std::max(0.0, 1.0 - (stats.distanceStdDev / maxAllowedDistanceVar));
    double accuracyConsistency = std::max(0.0, 1.0 - (stats.accuracyStdDev / maxAllowedAccuracyVar));

    stats.consistencyScore = (distanceConsistency + accuracyConsistency) / 2.0;

    return stats;
}

void ClubAnalysis::updateClubStatistics(const ShotData& shot) {
    auto clubProfile = storage_.getClubProfile(shot.clubUsed);
    if (!clubProfile) {
        ClubProfile newProfile;
        newProfile.name = shot.clubUsed;
        newProfile.avgDistance = shot.actualDistance;
        newProfile.avgSpinRate = shot.spinRate;
        newProfile.avgLaunchAngle = shot.launchAngle;
        newProfile.totalShots = 1;
        newProfile.lastUpdated = std::time(nullptr);
        storage_.saveClubProfile(newProfile);
        return;
    }

    ClubProfile updated = *clubProfile;
    double weight = 1.0 / (updated.totalShots + 1);
    
    updated.avgDistance = (updated.avgDistance * updated.totalShots + shot.actualDistance) * weight;
    updated.avgSpinRate = (updated.avgSpinRate * updated.totalShots + shot.spinRate) * weight;
    updated.avgLaunchAngle = (updated.avgLaunchAngle * updated.totalShots + shot.launchAngle) * weight;
    
    auto stats = analyzeClubPerformance(shot.clubUsed);
    updated.distanceDeviation = stats.distanceStdDev;
    updated.directionDeviation = stats.accuracyStdDev;
    
    updated.totalShots++;
    updated.lastUpdated = std::time(nullptr);
    
    storage_.updateClubProfile(updated);
}

std::map<std::string, std::pair<double, double>> ClubAnalysis::getOptimalDistanceRanges() {
    std::map<std::string, std::pair<double, double>> ranges;
    auto clubs = storage_.getAllClubProfiles();

    std::sort(clubs.begin(), clubs.end(),
              [](const ClubProfile& a, const ClubProfile& b) {
                  return a.avgDistance < b.avgDistance;
              });

    for (size_t i = 0; i < clubs.size(); ++i) {
        auto stats = analyzeClubPerformance(clubs[i].name);
        double minDist = clubs[i].avgDistance - 2 * stats.distanceStdDev;
        double maxDist = clubs[i].avgDistance + 2 * stats.distanceStdDev;

        if (i > 0) {
            // No need to store prevStats since we're not using it
            analyzeClubPerformance(clubs[i-1].name);
            double midpoint = (clubs[i].avgDistance + clubs[i-1].avgDistance) / 2;
            ranges[clubs[i-1].name].second = midpoint;
            minDist = midpoint;
        }
        
        ranges[clubs[i].name] = std::make_pair(minDist, maxDist);
    }

    return ranges;
}

double ClubAnalysis::calculateConfidenceScore(
    double targetDistance,
    const ClubProfile& profile,
    const weather::WeatherData& conditions
) {
    double adjustedDistance = adjustDistanceForConditions(profile.avgDistance, conditions);
    double distanceDiff = std::abs(targetDistance - adjustedDistance);
    double distanceConfidence = std::max(0.0, 1.0 - (distanceDiff / adjustedDistance));

    auto stats = analyzeClubPerformance(profile.name);
    double consistencyWeight = 0.3;
    
    return (distanceConfidence * (1.0 - consistencyWeight)) + 
           (stats.consistencyScore * consistencyWeight);
}

double ClubAnalysis::adjustDistanceForConditions(
    double baseDistance,
    const weather::WeatherData& conditions
) {
    double windEffect = conditions.windSpeed * std::cos(conditions.windDirection * PI / 180.0);
    double windAdjustment = windEffect * 0.9;

    double tempEffect = (conditions.temperature - 20.0) * 0.2;
    double pressureEffect = (conditions.pressure - 1013.25) * -0.1;

    return baseDistance + windAdjustment + tempEffect + pressureEffect;
}

std::vector<ShotData> ClubAnalysis::getRecentShots(
    const std::string& clubName,
    [[maybe_unused]] size_t limit
) {
    return storage_.getShotsByClub(clubName);
}

}} // namespace gptgolf::data
