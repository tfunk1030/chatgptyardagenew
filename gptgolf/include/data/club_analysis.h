#pragma once

#include <vector>
#include <map>
#include <optional>
#include "storage.h"

/**
 * @file club_analysis.h
 * @brief Club performance analysis and recommendation system
 *
 * This module provides functionality for analyzing golf club performance based on
 * historical shot data and making intelligent club recommendations based on
 * target distance and weather conditions. It uses statistical analysis to track
 * club consistency and optimal distance ranges.
 */

namespace gptgolf {
namespace data {

/**
 * @brief Club recommendation with confidence metrics
 *
 * Contains a specific club recommendation along with supporting data
 * and explanation for why this club was chosen.
 */
struct ClubRecommendation {
    std::string clubName;         //!< Name/identifier of recommended club
    double confidenceScore;       //!< Confidence in recommendation (0-1)
    double expectedDistance;      //!< Predicted carry distance (meters)
    double expectedAccuracy;      //!< Predicted lateral accuracy (meters)
    std::string reasoning;        //!< Human-readable explanation of recommendation

    /**
     * @brief Check if recommendation is highly confident
     * @return true if confidence score >= 0.8
     */
    bool isHighConfidence() const { return confidenceScore >= 0.8; }
};

/**
 * @brief Statistical analysis of club performance
 *
 * Aggregated statistics from historical shot data for a specific club,
 * including measures of both distance and accuracy consistency.
 */
struct ClubStatistics {
    double meanDistance;      //!< Average carry distance (meters)
    double distanceStdDev;   //!< Standard deviation of distances (meters)
    double meanAccuracy;      //!< Average lateral deviation from target (meters)
    double accuracyStdDev;   //!< Standard deviation of lateral deviation (meters)
    double consistencyScore; //!< Overall consistency rating (0-1)
    size_t sampleSize;       //!< Number of shots analyzed

    /**
     * @brief Check if statistics are statistically significant
     * @return true if sample size >= 10
     */
    bool isSignificant() const { return sampleSize >= 10; }
};

/**
 * @brief Club performance analysis and recommendation engine
 *
 * Analyzes historical shot data to track club performance metrics and
 * provide intelligent club selection recommendations based on target
 * distance and current conditions.
 */
class ClubAnalysis {
public:
    /**
     * @brief Construct a new Club Analysis object
     * @param storage Reference to storage interface for shot data
     */
    explicit ClubAnalysis(IStorage& storage);

    /**
     * @brief Get club recommendation for specific shot
     *
     * Analyzes historical performance data and current conditions to
     * recommend the optimal club for achieving the target distance.
     *
     * @param targetDistance Desired carry distance (meters)
     * @param conditions Current weather conditions
     * @return ClubRecommendation containing recommended club and supporting data
     *
     * The recommendation algorithm considers:
     * - Historical club performance
     * - Weather effects on carry distance
     * - Player consistency with each club
     * - Risk/reward tradeoffs
     */
    ClubRecommendation recommendClub(
        double targetDistance,
        const weather::WeatherData& conditions
    );

    /**
     * @brief Generate statistical analysis for specific club
     *
     * Analyzes historical shot data to generate performance metrics
     * for the specified club.
     *
     * @param clubName Name/identifier of club to analyze
     * @return ClubStatistics containing performance metrics
     *
     * Analysis includes:
     * - Distance consistency (mean and standard deviation)
     * - Directional accuracy
     * - Overall consistency rating
     * - Statistical significance check
     */
    ClubStatistics analyzeClubPerformance(const std::string& clubName);

    /**
     * @brief Update statistics with new shot data
     *
     * Incorporates new shot data into the historical performance
     * metrics for the relevant club.
     *
     * @param shot Shot data to incorporate
     *
     * @note This method automatically triggers recalculation of
     * affected statistics and optimal ranges.
     */
    void updateClubStatistics(const ShotData& shot);

    /**
     * @brief Get optimal distance ranges for all clubs
     *
     * Determines the optimal distance range for each club based on
     * historical performance data.
     *
     * @return Map of club names to their optimal distance ranges (min, max)
     *
     * Ranges are calculated to:
     * - Maximize consistency within each range
     * - Minimize gaps between ranges
     * - Account for overlap in capabilities
     */
    std::map<std::string, std::pair<double, double>> getOptimalDistanceRanges();

private:
    IStorage& storage_; //!< Reference to shot data storage

    /**
     * @brief Calculate confidence score for club recommendation
     *
     * @param targetDistance Desired carry distance (meters)
     * @param profile Club profile data
     * @param conditions Current weather conditions
     * @return Confidence score (0-1)
     */
    double calculateConfidenceScore(
        double targetDistance,
        const ClubProfile& profile,
        const weather::WeatherData& conditions
    );

    /**
     * @brief Adjust expected distance for weather conditions
     *
     * @param baseDistance Standard carry distance (meters)
     * @param conditions Current weather conditions
     * @return Adjusted carry distance (meters)
     */
    double adjustDistanceForConditions(
        double baseDistance,
        const weather::WeatherData& conditions
    );

    /**
     * @brief Retrieve recent shots with specific club
     *
     * @param clubName Name/identifier of club
     * @param limit Maximum number of shots to retrieve
     * @return Vector of recent shots with this club
     */
    std::vector<ShotData> getRecentShots(
        const std::string& clubName,
        size_t limit = 20
    );
};

} // namespace data
} // namespace gptgolf
