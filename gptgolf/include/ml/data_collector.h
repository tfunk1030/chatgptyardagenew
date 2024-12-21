#pragma once

#include <vector>
#include <map>
#include "../data/storage.h"
#include "../weather/weather_data.h"

/**
 * @file data_collector.h
 * @brief Shot data collection and pattern analysis
 *
 * This module handles the collection, validation, and analysis of golf shot data.
 * It processes raw shot data to identify patterns, analyze weather impacts,
 * and provide statistical insights for the machine learning models.
 */

namespace gptgolf {
namespace ml {

/**
 * @brief Analysis of shot pattern and accuracy
 *
 * Contains detailed analysis of a shot or series of shots,
 * including accuracy metrics and identified patterns.
 */
struct ShotPattern {
    double distanceError;      //!< Distance deviation from prediction (meters)
    double lateralError;       //!< Lateral deviation from target line (meters)
    double conditionImpact;    //!< Calculated weather impact factor (0-1)
    std::string pattern_type;  //!< Identified pattern category

    /**
     * @brief Check if pattern represents a significant deviation
     * @return true if errors exceed normal thresholds
     */
    bool isSignificant() const {
        return std::abs(distanceError) > 10.0 || std::abs(lateralError) > 5.0;
    }
};

/**
 * @brief Data collection and pattern analysis system
 *
 * Processes and analyzes golf shot data to identify patterns,
 * validate measurements, and provide statistical insights for
 * the machine learning models.
 */
class DataCollector {
public:
    /**
     * @brief Construct a new Data Collector
     * @param storage Reference to shot data storage system
     */
    explicit DataCollector(data::IStorage& storage);

    /** @name Data Processing
     * Methods for processing and analyzing shot data
     * @{
     */
    /**
     * @brief Process new shot data
     *
     * Analyzes a new shot to identify patterns and calculate
     * accuracy metrics. Performs validation and outlier detection
     * before processing.
     *
     * @param shot New shot data to process
     * @return ShotPattern containing analysis results
     *
     * Analysis includes:
     * - Distance accuracy
     * - Directional accuracy
     * - Weather impact assessment
     * - Pattern identification
     */
    ShotPattern processShotData(const data::ShotData& shot);

    /**
     * @brief Analyze patterns for specific club
     *
     * Examines historical shots with a specific club to identify
     * recurring patterns and tendencies.
     *
     * @param clubName Club to analyze
     * @param limit Maximum number of shots to analyze
     * @return Vector of identified patterns
     *
     * Patterns analyzed include:
     * - Consistent slice/hook tendencies
     * - Distance control patterns
     * - Weather sensitivity
     */
    std::vector<ShotPattern> analyzeClubPatterns(const std::string& clubName, size_t limit = 50);

    /**
     * @brief Calculate weather impact on shot
     *
     * Determines how much weather conditions affected a shot's outcome
     * compared to standard conditions.
     *
     * @param conditions Weather conditions during shot
     * @param shot Shot data to analyze
     * @return Impact factor (0-1, where 1 = maximum impact)
     */
    double calculateConditionImpact(
        const weather::WeatherData& conditions,
        const data::ShotData& shot
    );

    /**
     * @brief Get statistical analysis of patterns
     *
     * Provides aggregated statistics about shot patterns
     * for a specific club.
     *
     * @param clubName Club to analyze
     * @return Map of statistic names to values
     *
     * Statistics include:
     * - Pattern frequencies
     * - Average deviations
     * - Consistency metrics
     */
    std::map<std::string, double> getPatternStatistics(const std::string& clubName);

    /**
     * @brief Validate shot data
     *
     * Checks shot data for validity and reasonable values.
     *
     * @param shot Shot data to validate
     * @return true if data passes validation
     *
     * Validation checks:
     * - Value ranges
     * - Physical possibility
     * - Data completeness
     * - Sensor reliability
     */
    bool validateShotData(const data::ShotData& shot);
    /** @} */

private:
    data::IStorage& storage_; //!< Reference to shot data storage

    /** @name Helper Methods
     * Internal methods for data analysis
     * @{
     */
    /**
     * @brief Identify shot pattern type
     *
     * Analyzes shot errors and conditions to categorize
     * the shot pattern.
     *
     * @param distanceError Distance deviation (meters)
     * @param lateralError Lateral deviation (meters)
     * @param conditions Weather conditions
     * @return Pattern type identifier
     */
    std::string identifyPattern(
        double distanceError,
        double lateralError,
        const weather::WeatherData& conditions
    );

    /**
     * @brief Normalize error value
     *
     * Converts raw error to normalized value based on
     * expected variation.
     *
     * @param error Raw error value
     * @param expectedValue Expected value for normalization
     * @return Normalized error value
     */
    double normalizeError(double error, double expectedValue);

    /**
     * @brief Check if shot is statistical outlier
     *
     * Determines if a shot's characteristics are statistically
     * abnormal compared to historical data.
     *
     * @param shot Shot to check
     * @param history Historical shots for comparison
     * @return true if shot is an outlier
     */
    bool isOutlier(const data::ShotData& shot, const std::vector<data::ShotData>& history);
    /** @} */
};

} // namespace ml
} // namespace gptgolf
