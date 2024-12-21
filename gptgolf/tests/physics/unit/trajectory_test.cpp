#include <gtest/gtest.h>
#include "physics/physics.h"
#include "physics/trajectory.h"
#include "physics/physics_validation.h"

class TrajectoryTest : public ::testing::Test {
protected:
    // Test constants
    const double VALID_SPEED = TRACKMAN_DRIVER_SPEED;
    const double VALID_ANGLE = TRACKMAN_DRIVER_LAUNCH;
    const double VALID_SPIN = TRACKMAN_DRIVER_SPIN;
    const double VALID_WIND = 5.0;
    const double VALID_WIND_ANGLE = 0.0;

    void SetUp() override {
        // Common setup for all tests
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
    
    // Helper function to validate trajectory physics
    void validateTrajectoryPhysics(const TrajectoryResultWithStatus& result, 
                                 double initialSpeed, double launchAngle) {
        ASSERT_TRUE(result.isSuccess()) << "Trajectory calculation failed: " << result.errorMessage;
        ASSERT_TRUE(result.result.has_value());
        const auto& trajectory = *result.result;
        ASSERT_FALSE(trajectory.trajectory.empty());
        
        // Check initial conditions
        EXPECT_NEAR(trajectory.trajectory[0].x, 0.0, 0.001);
        EXPECT_NEAR(trajectory.trajectory[0].y, 0.0, 0.001);
        
        // Verify maximum height is recorded correctly
        double maxHeight = 0.0;
        for (const auto& point : trajectory.trajectory) {
            if (point.y > maxHeight) maxHeight = point.y;
        }
        EXPECT_NEAR(trajectory.apex, maxHeight, 0.001);
        
        // Check final point is at ground level
        EXPECT_NEAR(trajectory.trajectory.back().y, 0.0, 0.1);
    }
};

// Test basic trajectory calculation without wind
TEST_F(TrajectoryTest, BasicTrajectoryNoWind) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;  // Using TrackMan baseline data
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = 0.0;
    double windSpeed = 0.0;
    double windAngle = 0.0;

    TrajectoryResult result = calculateTrajectory(initialSpeed, launchAngle, spinRate, windSpeed, windAngle);
    validateTrajectoryPhysics(result, initialSpeed, launchAngle);

    // Basic distance check (rough approximation for ideal conditions)
    double expectedDistance = (initialSpeed * initialSpeed * sin(2 * launchAngle * M_PI/180.0)) / GRAVITY;
    EXPECT_NEAR(result.distance, expectedDistance, expectedDistance * 0.3); // 30% tolerance for air resistance
}

// Test trajectory with headwind
TEST_F(TrajectoryTest, TrajectoryWithHeadwind) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = 0.0;
    double windSpeed = 5.0;      // 5 m/s headwind
    double windAngle = 180.0;    // Directly opposing flight

    TrajectoryResult resultWithWind = calculateTrajectory(initialSpeed, launchAngle, spinRate, windSpeed, windAngle);
    TrajectoryResult resultNoWind = calculateTrajectory(initialSpeed, launchAngle, spinRate, 0.0, 0.0);

    validateTrajectoryPhysics(resultWithWind, initialSpeed, launchAngle);
    
    // Headwind should reduce total distance
    EXPECT_LT(resultWithWind.distance, resultNoWind.distance);
    
    // Headwind typically increases apex height
    EXPECT_GT(resultWithWind.apex, resultNoWind.apex);
}

// Test Magnus effect with TrackMan baseline data
TEST_F(TrajectoryTest, TrajectoryWithSpin) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    double windSpeed = 0.0;
    double windAngle = 0.0;

    TrajectoryResult resultWithSpin = calculateTrajectory(initialSpeed, launchAngle, spinRate, windSpeed, windAngle);
    TrajectoryResult resultNoSpin = calculateTrajectory(initialSpeed, launchAngle, 0.0, windSpeed, windAngle);

    validateTrajectoryPhysics(resultWithSpin, initialSpeed, launchAngle);
    
    // Backspin should increase apex height
    EXPECT_GT(resultWithSpin.apex, resultNoSpin.apex);
    
    // Backspin typically increases carry distance
    EXPECT_GT(resultWithSpin.distance, resultNoSpin.distance);
    
    // Verify apex height is within reasonable range for driver shot
    EXPECT_GT(resultWithSpin.apex, 20.0);  // At least 20m high
    EXPECT_LT(resultWithSpin.apex, 50.0);  // Not more than 50m high
    
    // Verify spin decay effects
    double timeAtApex = sqrt(2.0 * resultWithSpin.apex / GRAVITY); // Approximate time to apex
    double decayedSpin = calculateSpinDecay(spinRate, timeAtApex);
    EXPECT_LT(decayedSpin, spinRate); // Spin should decrease over time
    EXPECT_GT(decayedSpin, spinRate * 0.5); // But not too much for typical driver shot time
}

// Test spin axis effects on trajectory
TEST_F(TrajectoryTest, SpinAxisEffects) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    double windSpeed = 0.0;
    double windAngle = 0.0;
    
    // Test pure backspin (vertical spin axis)
    SpinAxis verticalAxis(0.0, 0.0);
    double magnusForceVertical = calculateMagnusForce(spinRate, initialSpeed, 
                                                    BALL_RADIUS, verticalAxis, 0.0);
    
    // Test tilted spin axis (simulating draw/fade)
    SpinAxis tiltedAxis(20.0, 0.0);
    double magnutsForceTilted = calculateMagnusForce(spinRate, initialSpeed,
                                                   BALL_RADIUS, tiltedAxis, 0.0);
    
    // Tilted axis should result in less vertical lift
    EXPECT_LT(magnutsForceTilted, magnusForceVertical);
    
    // Test lift coefficient calculation
    double liftCoef = calculateLiftCoefficient(spinRate, initialSpeed);
    EXPECT_GT(liftCoef, 0.0);
    EXPECT_LE(liftCoef, MAX_LIFT_COEFFICIENT);
}

// Test spin decay over time
TEST_F(TrajectoryTest, SpinDecayOverTime) {
    double initialSpin = TRACKMAN_DRIVER_SPIN;
    
    // Test at different time intervals
    double spin1s = calculateSpinDecay(initialSpin, 1.0);
    double spin2s = calculateSpinDecay(initialSpin, 2.0);
    double spin4s = calculateSpinDecay(initialSpin, 4.0);
    
    // Verify spin decreases over time
    EXPECT_LT(spin1s, initialSpin);
    EXPECT_LT(spin2s, spin1s);
    EXPECT_LT(spin4s, spin2s);
    
    // Verify decay rate is reasonable
    // Spin should not decay too quickly or too slowly
    EXPECT_GT(spin1s, initialSpin * 0.9);  // Not too fast in first second
    EXPECT_LT(spin4s, initialSpin * 0.5);  // Significant decay after 4 seconds
}

// Test Reynolds number effects
TEST_F(TrajectoryTest, ReynoldsNumberEffects) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    
    // Test at different altitudes
    TrajectoryResult resultSeaLevel = calculateTrajectory(initialSpeed, launchAngle, spinRate, 0.0, 0.0);
    
    // Simulate shot at high altitude (thinner air)
    double highAltitudeDensity = getAirDensity(nullptr, 2000.0); // 2000m elevation
    double seaLevelDensity = getAirDensity(nullptr, 0.0);
    EXPECT_LT(highAltitudeDensity, seaLevelDensity);
    
    // Reynolds number should be lower at high altitude
    double reSeaLevel = calculateReynoldsNumber(initialSpeed, 0.0);
    double reHighAlt = calculateReynoldsNumber(initialSpeed, 2000.0);
    EXPECT_LT(reHighAlt, reSeaLevel);
}

// Test wind gradient effects
TEST_F(TrajectoryTest, WindGradientEffects) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    double baseWindSpeed = 5.0;
    
    // Wind should be stronger at apex than at ground level
    double groundWind = getWindGradient(baseWindSpeed, 0.0);
    double apexWind = getWindGradient(baseWindSpeed, 30.0);  // Typical driver apex height
    EXPECT_GT(apexWind, groundWind);
}

// Test input validation
TEST_F(TrajectoryTest, InputValidation) {
    // Test negative initial speed
    auto result = calculateTrajectoryWithValidation(-1.0, VALID_ANGLE, VALID_SPIN, 
                                                  VALID_WIND, VALID_WIND_ANGLE);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "speed cannot be negative");

    // Test excessive initial speed
    result = calculateTrajectoryWithValidation(150.0, VALID_ANGLE, VALID_SPIN,
                                             VALID_WIND, VALID_WIND_ANGLE);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "speed exceeds maximum");

    // Test invalid launch angle
    result = calculateTrajectoryWithValidation(VALID_SPEED, 95.0, VALID_SPIN,
                                             VALID_WIND, VALID_WIND_ANGLE);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "angle must be between");

    // Test negative spin rate
    result = calculateTrajectoryWithValidation(VALID_SPEED, VALID_ANGLE, -100.0,
                                             VALID_WIND, VALID_WIND_ANGLE);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "spin rate cannot be negative");

    // Test excessive spin rate
    result = calculateTrajectoryWithValidation(VALID_SPEED, VALID_ANGLE, 15000.0,
                                             VALID_WIND, VALID_WIND_ANGLE);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "spin rate exceeds maximum");

    // Test negative wind speed
    result = calculateTrajectoryWithValidation(VALID_SPEED, VALID_ANGLE, VALID_SPIN,
                                             -5.0, VALID_WIND_ANGLE);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "wind speed cannot be negative");

    // Test excessive wind speed
    result = calculateTrajectoryWithValidation(VALID_SPEED, VALID_ANGLE, VALID_SPIN,
                                             60.0, VALID_WIND_ANGLE);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "wind speed exceeds maximum");

    // Test invalid wind angle
    result = calculateTrajectoryWithValidation(VALID_SPEED, VALID_ANGLE, VALID_SPIN,
                                             VALID_WIND, 400.0);
    validateErrorHandling(result, TrajectoryStatus::InvalidInput, "wind angle must be between");
}

// Test successful calculation
TEST_F(TrajectoryTest, SuccessfulCalculation) {
    auto result = calculateTrajectoryWithValidation(VALID_SPEED, VALID_ANGLE, VALID_SPIN,
                                                  VALID_WIND, VALID_WIND_ANGLE);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.result.has_value());
    EXPECT_EQ(result.status, TrajectoryStatus::Success);
    EXPECT_TRUE(result.errorMessage.empty());
    validateTrajectoryPhysics(result, VALID_SPEED, VALID_ANGLE);
}

// Test legacy function compatibility
TEST_F(TrajectoryTest, LegacyFunctionCompatibility) {
    // Test with valid inputs
    TrajectoryResult legacyResult = calculateTrajectory(VALID_SPEED, VALID_ANGLE, 
                                                      VALID_SPIN, VALID_WIND, VALID_WIND_ANGLE);
    EXPECT_FALSE(legacyResult.trajectory.empty());
    
    // Test with invalid inputs (should return empty trajectory)
    TrajectoryResult invalidResult = calculateTrajectory(-1.0, VALID_ANGLE,
                                                       VALID_SPIN, VALID_WIND, VALID_WIND_ANGLE);
    EXPECT_TRUE(invalidResult.trajectory.empty());
    EXPECT_EQ(invalidResult.distance, 0.0);
    EXPECT_EQ(invalidResult.apex, 0.0);
}

// Test adaptive timestep behavior
TEST_F(TrajectoryTest, AdaptiveTimestepBehavior) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    double windSpeed = 0.0;
    double windAngle = 0.0;

    TrajectoryResult result = calculateTrajectory(initialSpeed, launchAngle, spinRate, windSpeed, windAngle);

    // Analyze point spacing in different phases
    std::vector<double> launchPhaseSteps;    // First 10 points
    std::vector<double> midFlightSteps;      // Middle points
    std::vector<double> landingPhaseSteps;   // Last 10 points
    
    for (size_t i = 1; i < result.trajectory.size(); ++i) {
        double dx = result.trajectory[i].x - result.trajectory[i-1].x;
        double dy = result.trajectory[i].y - result.trajectory[i-1].y;
        double stepSize = sqrt(dx*dx + dy*dy);
        
        if (i <= 10) {
            launchPhaseSteps.push_back(stepSize);
        } else if (i >= result.trajectory.size() - 10) {
            landingPhaseSteps.push_back(stepSize);
        } else {
            midFlightSteps.push_back(stepSize);
        }
    }
    
    // Calculate average step sizes for each phase
    double avgLaunchStep = std::accumulate(launchPhaseSteps.begin(), 
                                         launchPhaseSteps.end(), 0.0) / launchPhaseSteps.size();
    double avgMidFlightStep = std::accumulate(midFlightSteps.begin(), 
                                            midFlightSteps.end(), 0.0) / midFlightSteps.size();
    double avgLandingStep = std::accumulate(landingPhaseSteps.begin(), 
                                          landingPhaseSteps.end(), 0.0) / landingPhaseSteps.size();
    
    // Verify phase-specific behavior
    EXPECT_LT(avgLaunchStep, avgMidFlightStep) 
        << "Launch phase should have smaller steps than mid-flight";
    EXPECT_LT(avgLandingStep, avgMidFlightStep) 
        << "Landing phase should have smaller steps than mid-flight";
    
    // Verify smooth transitions
    double maxStepChange = 0.0;
    for (size_t i = 2; i < result.trajectory.size(); ++i) {
        double prevStep = sqrt(pow(result.trajectory[i-1].x - result.trajectory[i-2].x, 2) +
                             pow(result.trajectory[i-1].y - result.trajectory[i-2].y, 2));
        double currStep = sqrt(pow(result.trajectory[i].x - result.trajectory[i-1].x, 2) +
                             pow(result.trajectory[i].y - result.trajectory[i-1].y, 2));
        maxStepChange = std::max(maxStepChange, std::abs(currStep - prevStep));
    }
    
    // Verify step changes are smooth
    EXPECT_LT(maxStepChange, 0.1) 
        << "Step size changes should be gradual";
}

// Test trajectory data points consistency
TEST_F(TrajectoryTest, TrajectoryPointsConsistency) {
    double initialSpeed = TRACKMAN_DRIVER_SPEED;
    double launchAngle = TRACKMAN_DRIVER_LAUNCH;
    double spinRate = TRACKMAN_DRIVER_SPIN;
    double windSpeed = 0.0;
    double windAngle = 0.0;

    TrajectoryResult result = calculateTrajectory(initialSpeed, launchAngle, spinRate, windSpeed, windAngle);

    // Check trajectory points are continuous and physically reasonable
    for (size_t i = 1; i < result.trajectory.size(); ++i) {
        double dx = result.trajectory[i].x - result.trajectory[i-1].x;
        double dy = result.trajectory[i].y - result.trajectory[i-1].y;
        double stepDistance = sqrt(dx*dx + dy*dy);
        
        // Each step should be reasonably small
        EXPECT_LT(stepDistance, 1.0) << "Large gap between points at index " << i;
        
        // Y coordinate should not be negative
        EXPECT_GE(result.trajectory[i].y, -0.001) << "Negative height at index " << i;
        
        // X coordinate should always increase (ball moving forward)
        EXPECT_GT(result.trajectory[i].x, result.trajectory[i-1].x);
    }
}
