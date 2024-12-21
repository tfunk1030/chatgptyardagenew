#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>
#include "physics/physics.h"

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Standard test parameters
        baseSpeed = 44.7;      // 100 mph
        launchAngle = 12.0;    // degrees
        spinRate = 2500.0;     // rpm
        windSpeed = 5.0;       // m/s
        windAngle = 45.0;      // degrees
    }

    // Helper function to measure execution time
    template<typename Func>
    double measureExecutionTime(Func func, int iterations = 1) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            func();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        return duration.count() / iterations;  // Average time per iteration in milliseconds
    }

    double baseSpeed;
    double launchAngle;
    double spinRate;
    double windSpeed;
    double windAngle;

    // Performance thresholds - tightened after optimizations
    const double MAX_TRAJECTORY_CALC_TIME = 2.0;    // 2ms for single trajectory
    const double MAX_BATCH_CALC_TIME = 25.0;        // 25ms for batch calculations
    const double MAX_WIND_CALC_TIME = 0.05;         // 0.05ms for wind calculations
    const double MAX_VALIDATION_TIME = 0.01;        // 0.01ms for input validation
    const size_t MAX_TRAJECTORY_POINTS = 500;       // Maximum points after decimation
};

// Test validation performance
TEST_F(PerformanceTest, ValidationPerformance) {
    // Test validation overhead
    auto validationFunc = [this]() {
        return calculateTrajectoryWithValidation(baseSpeed, launchAngle, spinRate, windSpeed, windAngle);
    };

    double avgValidationTime = measureExecutionTime(validationFunc, 1000);
    
    EXPECT_LE(avgValidationTime, MAX_TRAJECTORY_CALC_TIME + MAX_VALIDATION_TIME)
        << "Validation adds too much overhead: " << avgValidationTime << "ms";

    // Test validation with invalid inputs
    auto invalidFunc = [this]() {
        return calculateTrajectoryWithValidation(-1.0, launchAngle, spinRate, windSpeed, windAngle);
    };

    double avgInvalidTime = measureExecutionTime(invalidFunc, 1000);
    
    EXPECT_LE(avgInvalidTime, MAX_VALIDATION_TIME)
        << "Invalid input handling too slow: " << avgInvalidTime << "ms";
}

// Test error handling performance
TEST_F(PerformanceTest, ErrorHandlingPerformance) {
    std::vector<std::pair<double, double>> invalidInputs = {
        {-1.0, launchAngle},    // Invalid speed
        {baseSpeed, 91.0},      // Invalid angle
        {baseSpeed, launchAngle}, // Valid input for comparison
    };

    for (const auto& [speed, angle] : invalidInputs) {
        auto errorFunc = [this, speed, angle]() {
            return calculateTrajectoryWithValidation(speed, angle, spinRate, windSpeed, windAngle);
        };

        double avgTime = measureExecutionTime(errorFunc, 100);
        
        if (speed < 0.0 || angle > 90.0) {
            EXPECT_LE(avgTime, MAX_VALIDATION_TIME)
                << "Error handling for invalid input too slow: " << avgTime << "ms";
        } else {
            EXPECT_LE(avgTime, MAX_TRAJECTORY_CALC_TIME + MAX_VALIDATION_TIME)
                << "Valid input handling too slow: " << avgTime << "ms";
        }
    }
}

// Test single trajectory calculation performance
TEST_F(PerformanceTest, SingleTrajectoryPerformance) {
    auto calcFunc = [this]() {
        return calculateTrajectory(baseSpeed, launchAngle, spinRate, windSpeed, windAngle);
    };

    double avgTime = measureExecutionTime(calcFunc, 100);
    auto result = calcFunc();  // Get a result for point count verification
    
    EXPECT_LE(avgTime, MAX_TRAJECTORY_CALC_TIME)
        << "Single trajectory calculation took too long: " << avgTime << "ms";
        
    EXPECT_LE(result.trajectory.size(), MAX_TRAJECTORY_POINTS)
        << "Trajectory contains too many points after decimation: " << result.trajectory.size();
}

// Test adaptive timestep effectiveness
TEST_F(PerformanceTest, AdaptiveTimestepEfficiency) {
    // Test with different initial velocities
    std::vector<double> speeds = {20.0, 44.7, 70.0};  // Various speeds in m/s
    std::vector<size_t> pointCounts;
    std::vector<double> calcTimes;
    
    for (double speed : speeds) {
        auto calcFunc = [this, speed]() {
            return calculateTrajectory(speed, launchAngle, spinRate, windSpeed, windAngle);
        };
        
        double avgTime = measureExecutionTime(calcFunc, 50);
        auto result = calcFunc();
        
        pointCounts.push_back(result.trajectory.size());
        calcTimes.push_back(avgTime);
        
        // Higher speeds should have more points due to smaller timesteps
        if (speed > 44.7) {
            EXPECT_GT(result.trajectory.size(), pointCounts[0])
                << "Higher speed should result in more trajectory points";
        }
    }
    
    // Verify calculation time scales reasonably with speed
    EXPECT_LE(calcTimes[2] / calcTimes[0], 2.0)
        << "High-speed calculations taking too much longer than low-speed ones";
}

// Test batch trajectory calculations performance
TEST_F(PerformanceTest, BatchTrajectoryPerformance) {
    // Test calculating multiple trajectories in sequence
    const int batchSize = 50;  // Simulate calculating trajectories for multiple clubs/conditions
    
    auto batchCalcFunc = [this, batchSize]() {
        std::vector<TrajectoryResult> results;
        results.reserve(batchSize);
        
        // Vary parameters slightly for each calculation
        for (int i = 0; i < batchSize; ++i) {
            double speedVar = baseSpeed + (i - batchSize/2) * 0.5;  // Vary speed
            double angleVar = launchAngle + (i - batchSize/2) * 0.2;  // Vary angle
            results.push_back(calculateTrajectory(speedVar, angleVar, spinRate, windSpeed, windAngle));
        }
        return results;
    };

    double avgTime = measureExecutionTime(batchCalcFunc);
    
    EXPECT_LE(avgTime, MAX_BATCH_CALC_TIME)
        << "Batch trajectory calculations took too long: " << avgTime << "ms";
}

// Test wind calculations performance
TEST_F(PerformanceTest, WindCalculationsPerformance) {
    auto windCalcFunc = [this]() {
        double relativeVelX, relativeVelY;
        calculateRelativeVelocity(baseSpeed, launchAngle, windSpeed, windAngle, relativeVelX, relativeVelY);
    };

    double avgTime = measureExecutionTime(windCalcFunc, 1000);
    
    EXPECT_LE(avgTime, MAX_WIND_CALC_TIME)
        << "Wind calculations took too long: " << avgTime << "ms";
}

// Test memory usage and point decimation
TEST_F(PerformanceTest, MemoryUsageAndDecimation) {
    std::vector<size_t> pointCounts;
    std::vector<double> distances;
    const int numTests = 100;
    
    for (int i = 0; i < numTests; ++i) {
        TrajectoryResult result = calculateTrajectory(
            baseSpeed + (i - numTests/2) * 0.2,  // Vary speed slightly
            launchAngle,
            spinRate,
            windSpeed,
            windAngle
        );
        pointCounts.push_back(result.trajectory.size());
        distances.push_back(result.distance);
    }

    // Calculate statistics
    double avgPoints = std::accumulate(pointCounts.begin(), pointCounts.end(), 0.0) / numTests;
    size_t maxPoints = *std::max_element(pointCounts.begin(), pointCounts.end());
    
    // Calculate distance variation
    double avgDistance = std::accumulate(distances.begin(), distances.end(), 0.0) / numTests;
    double maxDistanceVariation = 0.0;
    for (double dist : distances) {
        maxDistanceVariation = std::max(maxDistanceVariation, std::abs(dist - avgDistance));
    }
    
    // Verify memory usage and accuracy
    EXPECT_LE(maxPoints, MAX_TRAJECTORY_POINTS)
        << "Trajectory calculation using too many points after decimation: " << maxPoints;
    
    EXPECT_LE(avgPoints * sizeof(TrajectoryPoint), 4096)  // 4KB average memory per trajectory
        << "Average memory usage per trajectory too high: " << avgPoints * sizeof(TrajectoryPoint) << " bytes";
        
    EXPECT_LE(maxDistanceVariation, 0.1)  // Ensure decimation doesn't affect accuracy significantly
        << "Distance variation too high after decimation: " << maxDistanceVariation << "m";
}

// Test calculation stability under stress
TEST_F(PerformanceTest, StressTest) {
    const int numIterations = 1000;
    std::vector<double> distances;
    distances.reserve(numIterations);
    
    // Perform many calculations with varying parameters
    for (int i = 0; i < numIterations; ++i) {
        // Vary parameters within reasonable ranges
        double speed = baseSpeed + (rand() % 21 - 10);     // ±10 m/s
        double angle = launchAngle + (rand() % 11 - 5);    // ±5 degrees
        double spin = spinRate + (rand() % 1001 - 500);    // ±500 rpm
        double wind = windSpeed + (rand() % 11 - 5);       // ±5 m/s
        double windDir = windAngle + (rand() % 361);       // 0-360 degrees
        
        TrajectoryResult result = calculateTrajectory(speed, angle, spin, wind, windDir);
        distances.push_back(result.distance);
    }
    
    // Calculate statistics
    double sum = std::accumulate(distances.begin(), distances.end(), 0.0);
    double mean = sum / distances.size();
    
    std::vector<double> diff(distances.size());
    std::transform(distances.begin(), distances.end(), diff.begin(),
                  [mean](double x) { return x - mean; });
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / distances.size());
    
    // Verify results are stable (standard deviation should be reasonable given input variations)
    EXPECT_GT(stdev, 1.0) << "Standard deviation suspiciously low";
    EXPECT_LT(stdev, 50.0) << "Standard deviation suspiciously high";
}
