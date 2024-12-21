#include "../../include/ml/prediction_model.h"
#include <cmath>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace gptgolf {
namespace ml {

PredictionModel::PredictionModel(data::IStorage& storage, DataCollector& collector)
    : storage_(storage), collector_(collector) {
    // Initialize default weights
    conditionWeights_ = {
        {"wind_speed", 0.3},
        {"wind_direction", 0.2},
        {"temperature", 0.15},
        {"humidity", 0.1},
        {"pressure", 0.25}
    };
}

PredictionResult PredictionModel::predictShot(
    const std::string& clubName,
    const weather::WeatherData& conditions,
    double swingSpeed
) {
    PredictionResult result;
    auto features = extractFeatures(clubName, conditions, swingSpeed);
    
    // Get club profile
    auto clubProfile = storage_.getClubProfile(clubName);
    if (!clubProfile) {
        throw std::runtime_error("Club profile not found: " + clubName);
    }

    // Calculate base distance
    double baseDistance = calculateBaseDistance(clubName, swingSpeed);
    
    // Apply weather adjustments
    result.predictedDistance = adjustForConditions(baseDistance, conditions);
    
    // Calculate lateral prediction based on historical patterns
    auto patterns = collector_.analyzeClubPatterns(clubName);
    double avgLateralError = 0.0;
    if (!patterns.empty()) {
        avgLateralError = std::accumulate(patterns.begin(), patterns.end(), 0.0,
            [](double sum, const ShotPattern& p) { return sum + p.lateralError; }
        ) / patterns.size();
    }
    result.predictedLateral = avgLateralError;

    // Calculate confidence
    result.confidence = calculateConfidence(clubName, conditions);

    // Determine influential factors
    result.factors.clear();
    if (std::abs(conditions.windSpeed) > 5.0) {
        result.factors.push_back("Strong wind");
    }
    if (std::abs(conditions.temperature - 20.0) > 10.0) {
        result.factors.push_back("Temperature variation");
    }
    if (conditions.humidity > 70.0) {
        result.factors.push_back("High humidity");
    }

    return result;
}

void PredictionModel::train(const std::vector<data::ShotData>& trainingData) {
    if (trainingData.size() < minTrainingSize_) {
        throw std::runtime_error("Insufficient training data");
    }

    // Group shots by club
    std::map<std::string, std::vector<data::ShotData>> clubShots;
    for (const auto& shot : trainingData) {
        clubShots[shot.clubUsed].push_back(shot);
    }

    // Train for each club
    for (const auto& [clubName, shots] : clubShots) {
        std::vector<double> weights(5, 1.0); // Initialize weights
        
        // Simple gradient descent
        for (size_t epoch = 0; epoch < 100; ++epoch) {
            double totalError = 0.0;
            
            for (const auto& shot : shots) {
                auto features = extractFeatures(clubName, shot.conditions, shot.initialVelocity);
                double predicted = std::inner_product(
                    weights.begin(), weights.end(),
                    features.begin(), 0.0
                );
                
                double error = shot.actualDistance - predicted;
                totalError += error * error;

                // Update weights
                for (size_t i = 0; i < weights.size(); ++i) {
                    weights[i] += learningRate_ * error * features[i];
                }
            }

            // Early stopping if error is small enough
            if (totalError / shots.size() < 0.01) break;
        }

        clubWeights_[clubName] = weights;
    }
}

void PredictionModel::updateModel(const data::ShotData& newShot) {
    // Get existing shots for the club
    auto shots = storage_.getShotsByClub(newShot.clubUsed);
    shots.push_back(newShot);
    
    // Retrain if we have enough data
    if (shots.size() >= minTrainingSize_) {
        train(shots);
    }
}

double PredictionModel::evaluateAccuracy(const std::vector<data::ShotData>& testData) {
    if (testData.empty()) return 0.0;

    double totalError = 0.0;
    for (const auto& shot : testData) {
        auto prediction = predictShot(shot.clubUsed, shot.conditions, shot.initialVelocity);
        double error = std::abs(prediction.predictedDistance - shot.actualDistance);
        totalError += error * error;
    }

    return std::sqrt(totalError / testData.size()); // RMSE
}

std::map<std::string, double> PredictionModel::getModelMetrics() {
    std::map<std::string, double> metrics;
    
    // Calculate overall model performance
    auto allShots = storage_.getShotHistory();
    metrics["rmse"] = evaluateAccuracy(allShots);
    
    // Calculate per-club accuracy
    auto clubs = storage_.getAllClubProfiles();
    for (const auto& club : clubs) {
        auto clubShots = storage_.getShotsByClub(club.name);
        if (!clubShots.empty()) {
            metrics["club_" + club.name + "_rmse"] = evaluateAccuracy(clubShots);
        }
    }

    return metrics;
}

bool PredictionModel::saveModelState(const std::string& filepath) {
    try {
        std::ofstream file(filepath, std::ios::binary);
        
        // Save club weights
        size_t numClubs = clubWeights_.size();
        file.write(reinterpret_cast<const char*>(&numClubs), sizeof(numClubs));
        
        for (const auto& [club, weights] : clubWeights_) {
            size_t nameLen = club.length();
            file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            file.write(club.c_str(), nameLen);
            
            size_t numWeights = weights.size();
            file.write(reinterpret_cast<const char*>(&numWeights), sizeof(numWeights));
            file.write(reinterpret_cast<const char*>(weights.data()), numWeights * sizeof(double));
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool PredictionModel::loadModelState(const std::string& filepath) {
    try {
        std::ifstream file(filepath, std::ios::binary);
        
        // Load club weights
        size_t numClubs;
        file.read(reinterpret_cast<char*>(&numClubs), sizeof(numClubs));
        
        clubWeights_.clear();
        for (size_t i = 0; i < numClubs; ++i) {
            size_t nameLen;
            file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            
            std::string clubName(nameLen, '\0');
            file.read(&clubName[0], nameLen);
            
            size_t numWeights;
            file.read(reinterpret_cast<char*>(&numWeights), sizeof(numWeights));
            
            std::vector<double> weights(numWeights);
            file.read(reinterpret_cast<char*>(weights.data()), numWeights * sizeof(double));
            
            clubWeights_[clubName] = weights;
        }

        return true;
    } catch (...) {
        return false;
    }
}

double PredictionModel::calculateBaseDistance(const std::string& clubName, double swingSpeed) {
    auto clubProfile = storage_.getClubProfile(clubName);
    if (!clubProfile) {
        throw std::runtime_error("Club profile not found");
    }

    double baseDistance = clubProfile->avgDistance;
    
    // Adjust for swing speed if provided
    if (swingSpeed > 0.0) {
        // Simple linear adjustment based on swing speed
        double avgSwingSpeed = 100.0; // baseline
        double speedFactor = swingSpeed / avgSwingSpeed;
        baseDistance *= speedFactor;
    }

    return baseDistance;
}

double PredictionModel::adjustForConditions(double baseDistance, const weather::WeatherData& conditions) {
    double adjustment = 1.0;

    // Wind adjustment
    double windEffect = conditions.windSpeed * std::cos(conditions.windDirection) * 0.02;
    adjustment += windEffect;

    // Temperature adjustment (simplified)
    double tempEffect = (conditions.temperature - 20.0) * 0.001; // 20°C as baseline
    adjustment += tempEffect;

    // Humidity adjustment (simplified)
    double humidityEffect = (conditions.humidity - 50.0) * 0.0005; // 50% as baseline
    adjustment += humidityEffect;

    return baseDistance * adjustment;
}

double PredictionModel::calculateConfidence(
    const std::string& clubName,
    const weather::WeatherData& conditions
) {
    double confidence = 1.0;

    // Reduce confidence based on extreme conditions
    if (std::abs(conditions.windSpeed) > 20.0) confidence *= 0.8;
    if (std::abs(conditions.temperature - 20.0) > 15.0) confidence *= 0.9;
    if (conditions.humidity > 80.0) confidence *= 0.9;

    // Reduce confidence if we have limited data
    auto shots = storage_.getShotsByClub(clubName);
    if (shots.size() < minTrainingSize_) {
        confidence *= static_cast<double>(shots.size()) / minTrainingSize_;
    }

    return std::max(0.1, std::min(1.0, confidence));
}

std::vector<double> PredictionModel::extractFeatures(
    [[maybe_unused]] const std::string& clubName,
    const weather::WeatherData& conditions,
    double swingSpeed
) {
    std::vector<double> features;
    features.reserve(5);

    // Normalize features
    features.push_back(conditions.windSpeed / 30.0);  // Normalize wind speed (0-30 mph range)
    features.push_back(std::cos(conditions.windDirection));  // Wind direction as cosine
    features.push_back((conditions.temperature - 10.0) / 30.0);  // Normalize temp (10-40°C range)
    features.push_back(conditions.humidity / 100.0);  // Humidity already 0-100
    features.push_back(swingSpeed / 120.0);  // Normalize swing speed (0-120 mph range)

    return features;
}

} // namespace ml
} // namespace gptgolf
