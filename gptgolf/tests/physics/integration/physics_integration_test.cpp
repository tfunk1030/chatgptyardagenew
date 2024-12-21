#include <gtest/gtest.h>
#include "physics/physics.h"
#include "physics/physics_validation.h"
#include "weather/weather_data.h"

class PhysicsIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        weatherData.temperature = 20.0;    // 20°C
        weatherData.humidity = 50.0;       // 50%
        weatherData.pressure = 1013.25;    // Standard pressure
    }

    // Helper function to validate error handling
    void validateErrorHandling(TrajectoryResultWithStatus result,
                             TrajectoryStatus expectedStatus,
                             const std::string& expectedErrorSubstring) {
        EXPECT_EQ(result.status, expectedStatus);
        EXPECT_FALSE(result.isSuccess());
        EXPECT_FALSE(result.result.has_value());
        EXPECT_TRUE(result.errorMessage.find(expectedErrorSubstring) != std::string::npos)
            << "Expected error message containing '" << expectedErrorSubstring
            << "' but got: '" << result.errorMessage << "'";
    }

    WeatherData weatherData;
};

// Test error handling with weather conditions
TEST_F(PhysicsIntegrationTest, WeatherConditionsValidation) {
    // Test extreme temperature
    weatherData.temperature = -50.0;  // Extremely cold
    auto result = calculateTrajectoryWithValidation(
        TRACKMAN_DRIVER_SPEED,
        TRACKMAN_DRIVER_LAUNCH,
        TRACKMAN_DRIVER_SPIN,
        0.0, 0.0
    );
    validateErrorHandling(result, TrajectoryStatus::CalculationError, "Air density");

    // Test extreme pressure
    weatherData.temperature = 20.0;
    weatherData.pressure = 500.0;  // Extremely low pressure
    result = calculateTrajectoryWithValidation(
        TRACKMAN_DRIVER_SPEED,
        TRACKMAN_DRIVER_LAUNCH,
        TRACKMAN_DRIVER_SPIN,
        0.0, 0.0
    );
    validateErrorHandling(result, TrajectoryStatus::CalculationError, "Air density");
}

// Test convergence failure detection
TEST_F(PhysicsIntegrationTest, ConvergenceFailure) {
    // Test with extreme conditions that might cause convergence issues
    auto result = calculateTrajectoryWithValidation(
        100.0,  // Very high speed
        89.0,   // Nearly vertical
        10000.0, // Extreme spin
        30.0,    // Strong wind
        45.0
    );
    EXPECT_EQ(result.status, TrajectoryStatus::ConvergenceFailure);
}

// Test TrackMan baseline data validation
TEST_F(PhysicsIntegrationTest, TrackManValidation) {
    // Use TrackMan 2024 baseline data
    auto result = calculateTrajectoryWithValidation(
        TRACKMAN_DRIVER_SPEED,
        TRACKMAN_DRIVER_LAUNCH,
        TRACKMAN_DRIVER_SPIN,
        0.0,  // No wind for baseline
        0.0
    );

    // Verify successful calculation
    ASSERT_TRUE(result.isSuccess()) << "Calculation failed: " << result.errorMessage;
    ASSERT_TRUE(result.result.has_value());
    const auto& trajectory = *result.result;
    
    // TrackMan driver carry expectations (with tolerances)
    const double EXPECTED_CARRY = 275.0;  // ~275 yards
    const double CARRY_TOLERANCE = 10.0;  // ±10 yards
    
    // Convert meters to yards for comparison
    double carryYards = trajectory.distance * 1.09361;  // meters to yards
    EXPECT_NEAR(carryYards, EXPECTED_CARRY, CARRY_TOLERANCE);
    
    // Verify apex height is within TrackMan norms
    const double EXPECTED_APEX = 32.0;  // ~32 meters
    EXPECT_NEAR(trajectory.apex, EXPECTED_APEX, 5.0);

    // Verify physical constraints
    for (const auto& point : trajectory.trajectory) {
        // Check for NaN or infinity
        EXPECT_TRUE(std::isfinite(point.x)) << "Non-finite x coordinate detected";
        EXPECT_TRUE(std::isfinite(point.y)) << "Non-finite y coordinate detected";
        
        // Check physical bounds
        EXPECT_GE(point.x, 0.0) << "Negative x coordinate detected";
        EXPECT_LE(point.y, TRACKMAN_DRIVER_HEIGHT) << "Unrealistic height detected";
    }
}

// Test Reynolds number and drag crisis effects
TEST_F(PhysicsIntegrationTest, ReynoldsDragCrisis) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    
    // Test at different altitudes to vary Reynolds number
    std::vector<double> altitudes = {0.0, 1000.0, 2000.0};
    std::vector<TrajectoryResult> results;
    
    for (double altitude : altitudes) {
        // Adjust air density for altitude
        double density = getAirDensity(&weatherData, altitude);
        double re = calculateReynoldsNumber(initialSpeed, altitude);
        double cd = calculateDragCoefficient(re);
        
        TrajectoryResult result = calculateTrajectory(initialSpeed, launchAngle, spinRate, 0.0, 0.0);
        results.push_back(result);
        
        // Verify Reynolds number effects
        if (re > TURBULENT_REYNOLDS) {
            // Should be in drag crisis regime
            EXPECT_LT(cd, BASE_DRAG_COEFFICIENT * 0.6);
        }
    }
    
    // Higher altitude (lower Reynolds number) should result in different trajectory
    EXPECT_NE(results[0].distance, results[2].distance);
}

// Test Magnus effect with altitude
TEST_F(PhysicsIntegrationTest, MagnusAltitudeEffect) {
    // Test high spin shot at different altitudes
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = 4000.0;  // High spin rate
    
    TrajectoryResult seaLevelResult = calculateTrajectory(initialSpeed, launchAngle, spinRate, 0.0, 0.0);
    
    // Simulate at high altitude
    weatherData.pressure = 850.0;  // Approximately 1500m elevation
    double altitudeWindSpeed = getWindAdjustedSpeed(0.0, &weatherData);
    TrajectoryResult altitudeResult = calculateTrajectory(initialSpeed, launchAngle, spinRate, altitudeWindSpeed, 0.0);
    
    // Magnus effect should be reduced at altitude due to lower air density
    EXPECT_LT(altitudeResult.apex, seaLevelResult.apex);
}

// Test wind gradient effects
TEST_F(PhysicsIntegrationTest, WindGradientIntegration) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    double baseWindSpeed = 5.0;
    
    // Test with constant wind (old model)
    TrajectoryResult constantWindResult = calculateTrajectory(
        initialSpeed, launchAngle, spinRate,
        baseWindSpeed, 180.0  // Headwind
    );
    
    // Verify wind gradient implementation
    double groundWind = getWindGradient(baseWindSpeed, 0.0);
    double apexWind = getWindGradient(baseWindSpeed, 30.0);  // Typical apex height
    EXPECT_GT(apexWind, groundWind);
    
    // The apex height should be affected by the wind gradient
    EXPECT_GT(constantWindResult.apex, 20.0);  // Minimum expected apex
    EXPECT_LT(constantWindResult.apex, 40.0);  // Maximum expected apex
}

// Test complete shot simulation with all physics components
TEST_F(PhysicsIntegrationTest, CompleteShot) {
    // Use TrackMan baseline data with wind
    double windSpeed = 5.0;        // m/s (~11 mph)
    double windAngle = 45.0;       // degrees (quartering wind)

    TrajectoryResult result = calculateTrajectory(
        TRACKMAN_DRIVER_SPEED,
        TRACKMAN_DRIVER_LAUNCH,
        TRACKMAN_DRIVER_SPIN,
        windSpeed,
        windAngle
    );

    // Verify basic trajectory properties
    ASSERT_FALSE(result.trajectory.empty());
    EXPECT_GT(result.distance, 200.0);  // Minimum expected distance in meters
    EXPECT_LT(result.distance, 300.0);  // Maximum expected distance in meters
    
    // Verify reasonable apex height
    EXPECT_GT(result.apex, 20.0);
    EXPECT_LT(result.apex, 50.0);
    
    // Verify trajectory starts and ends at ground level
    EXPECT_NEAR(result.trajectory.front().y, 0.0, 0.001);
    EXPECT_NEAR(result.trajectory.back().y, 0.0, 0.1);
}

// Test temperature effects on ball flight
TEST_F(PhysicsIntegrationTest, TemperatureEffects) {
    // Test temperature effects on air density and trajectory
    std::vector<double> temperatures = {0.0, 20.0, 40.0};  // Cold, normal, hot
    std::vector<TrajectoryResult> results;
    
    for (double temp : temperatures) {
        weatherData.temperature = temp;
        double density = getAirDensity(&weatherData);
        
        TrajectoryResult result = calculateTrajectory(
            TRACKMAN_DRIVER_SPEED,
            TRACKMAN_DRIVER_LAUNCH,
            TRACKMAN_DRIVER_SPIN,
            0.0, 0.0  // No wind
        );
        results.push_back(result);
    }
    
    // Cold air (denser) should result in shorter shots
    EXPECT_LT(results[0].distance, results[1].distance);
    // Hot air (less dense) should result in longer shots
    EXPECT_GT(results[2].distance, results[1].distance);
}
