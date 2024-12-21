#pragma once

#include <vector>
#include <memory>
#include "../data/storage.h"
#include "../weather/weather_data.h"
#include "data_collector.h"

/**
 * @file prediction_model.h
 * @brief Machine learning model for golf shot prediction
 *
 * This module implements a machine learning model for predicting golf shot
 * outcomes based on club selection, weather conditions, and player characteristics.
 * The model continuously learns from new shot data to improve its predictions
 * and adapts to changes in player performance.
 */

namespace gptgolf {
namespace ml {

/**
 * @brief Result of shot prediction calculations
 *
 * Contains predicted outcomes and confidence metrics for a shot
 * prediction, along with explanatory factors.
 */
struct PredictionResult {
    double predictedDistance;      //!< Predicted carry distance (meters)
    double predictedLateral;       //!< Predicted lateral deviation (meters, positive = right)
    double confidence;             //!< Prediction confidence score (0-1)
    std::vector<std::string> factors; //!< Factors influencing the prediction

    /**
     * @brief Check if prediction is highly confident
     * @return true if confidence >= 0.8
     */
    bool isHighConfidence() const { return confidence >= 0.8; }
};

/**
 * @brief Machine learning model for shot prediction
 *
 * Implements a trainable model that predicts shot outcomes based on
 * historical data, current conditions, and player characteristics.
 * The model uses a combination of physics-based calculations and
 * machine learning techniques to generate predictions.
 */
class PredictionModel {
public:
    /**
     * @brief Construct a new Prediction Model
     * @param storage Reference to shot data storage
     * @param collector Reference to data collection system
     */
    explicit PredictionModel(data::IStorage& storage, DataCollector& collector);
    virtual ~PredictionModel() = default;

    /** @name Core Prediction Methods
     * Primary methods for shot prediction
     * @{
     */
    /**
     * @brief Predict outcome of a shot
     *
     * Generates a prediction for shot distance and accuracy based on
     * club selection, weather conditions, and optional swing speed.
     *
     * @param clubName Name/identifier of club to be used
     * @param conditions Current weather conditions
     * @param swingSpeed Optional known swing speed (m/s)
     * @return PredictionResult containing predicted outcomes
     *
     * The prediction algorithm considers:
     * - Historical performance with the selected club
     * - Current weather effects
     * - Player skill level and consistency
     * - Recent performance trends
     */
    virtual PredictionResult predictShot(
        const std::string& clubName,
        const weather::WeatherData& conditions,
        double swingSpeed = 0.0
    );
    /** @} */

    /** @name Model Training
     * Methods for training and updating the model
     * @{
     */
    /**
     * @brief Train model on historical shot data
     *
     * Performs batch training on a set of historical shots to
     * establish baseline model parameters.
     *
     * @param trainingData Vector of historical shot data
     * @throws std::runtime_error if training data is insufficient
     */
    virtual void train(const std::vector<data::ShotData>& trainingData);

    /**
     * @brief Update model with new shot data
     *
     * Incrementally updates model parameters based on a new shot,
     * allowing the model to adapt to changes in player performance.
     *
     * @param newShot New shot data to incorporate
     */
    virtual void updateModel(const data::ShotData& newShot);
    /** @} */

    /** @name Model Evaluation
     * Methods for assessing model performance
     * @{
     */
    /**
     * @brief Evaluate model accuracy
     *
     * Tests model predictions against actual outcomes in test data.
     *
     * @param testData Vector of shots to test against
     * @return Accuracy score (0-1)
     */
    virtual double evaluateAccuracy(const std::vector<data::ShotData>& testData);

    /**
     * @brief Get detailed model performance metrics
     * @return Map of metric names to values
     *
     * Available metrics include:
     * - Mean absolute error (meters)
     * - R-squared value
     * - Prediction bias
     * - Cross-validation score
     */
    virtual std::map<std::string, double> getModelMetrics();
    /** @} */

    /** @name Model Persistence
     * Methods for saving and loading model state
     * @{
     */
    /**
     * @brief Save model parameters to file
     * @param filepath Path to save file
     * @return true if save successful
     */
    virtual bool saveModelState(const std::string& filepath);

    /**
     * @brief Load model parameters from file
     * @param filepath Path to model file
     * @return true if load successful
     */
    virtual bool loadModelState(const std::string& filepath);
    /** @} */

protected:
    data::IStorage& storage_;      //!< Reference to shot data storage
    DataCollector& collector_;     //!< Reference to data collection system

    /** @name Helper Methods
     * Protected methods for internal calculations
     * @{
     */
    /**
     * @brief Calculate base distance for club
     *
     * @param clubName Club identifier
     * @param swingSpeed Optional known swing speed
     * @return Base carry distance in meters
     */
    virtual double calculateBaseDistance(
        const std::string& clubName,
        double swingSpeed
    );

    /**
     * @brief Adjust distance for weather conditions
     *
     * @param baseDistance Base carry distance
     * @param conditions Current weather conditions
     * @return Adjusted distance in meters
     */
    virtual double adjustForConditions(
        double baseDistance,
        const weather::WeatherData& conditions
    );

    /**
     * @brief Calculate prediction confidence
     *
     * @param clubName Club identifier
     * @param conditions Current weather conditions
     * @return Confidence score (0-1)
     */
    virtual double calculateConfidence(
        const std::string& clubName,
        const weather::WeatherData& conditions
    );

    /**
     * @brief Extract features for model input
     *
     * @param clubName Club identifier
     * @param conditions Weather conditions
     * @param swingSpeed Optional swing speed
     * @return Vector of feature values
     */
    virtual std::vector<double> extractFeatures(
        const std::string& clubName,
        const weather::WeatherData& conditions,
        double swingSpeed
    );
    /** @} */

    /** @name Model Parameters
     * Internal model state and configuration
     * @{
     */
    std::map<std::string, std::vector<double>> clubWeights_;    //!< Club-specific model weights
    std::map<std::string, double> conditionWeights_;            //!< Weather condition weights
    double learningRate_ = 0.01;                                //!< Model learning rate
    size_t minTrainingSize_ = 20;                               //!< Minimum required training samples
    /** @} */
};

} // namespace ml
} // namespace gptgolf
