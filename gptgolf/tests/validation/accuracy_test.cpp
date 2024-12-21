#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include "physics/physics.h"

struct ReferenceShot {
    double clubSpeed;      // m/s
    double launchAngle;    // degrees
    double spinRate;       // rpm
    double expectedCarry;  // meters
    double expectedApex;   // meters
    std::string clubType;
};

class AccuracyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reference data from TrackMan 2024 measurements
        // Speeds converted from mph to m/s
        // Distances converted from yards to meters
        referenceShots = {
            // Driver shots (TrackMan 2024 baseline)
            {TRACKMAN_DRIVER_SPEED, TRACKMAN_DRIVER_LAUNCH, TRACKMAN_DRIVER_SPIN, 275.0, TRACKMAN_DRIVER_HEIGHT, "Driver"},
            
            // Additional driver variations
            {51.4, 10.9, 2700, 247.0, 30.5, "Driver"},    // 115mph driver
            {46.9, 11.2, 2600, 229.0, 28.0, "Driver"},    // 105mph driver
            {42.5, 11.5, 2500, 210.0, 26.0, "Driver"},    // 95mph driver
            
            // 7-iron shots
            {34.0, 16.0, 6500, 155.0, 29.0, "7-Iron"},    // 76mph 7-iron
            {31.3, 16.5, 6300, 146.0, 27.5, "7-Iron"},    // 70mph 7-iron
            {28.6, 17.0, 6100, 137.0, 26.0, "7-Iron"},    // 64mph 7-iron
            
            // Wedge shots
            {25.0, 24.0, 8500, 110.0, 25.0, "Wedge"},     // 56mph wedge
            {22.4, 25.0, 8300, 101.0, 23.5, "Wedge"},     // 50mph wedge
            {19.7, 26.0, 8100, 91.0, 22.0, "Wedge"}       // 44mph wedge
        };
    }

    // Helper function to calculate percentage error
    double calculateError(double expected, double actual) {
        return std::abs(expected - actual) / expected * 100.0;
    }

    std::vector<ReferenceShot> referenceShots;
    const double MAX_CARRY_ERROR_PERCENT = 5.0;   // 5% tolerance for carry distance
    const double MAX_APEX_ERROR_PERCENT = 10.0;   // 10% tolerance for apex height
};

// Test input validation accuracy
TEST_F(AccuracyTest, InputValidationAccuracy) {
    struct ValidationTest {
        double speed;
        double angle;
        double spin;
        TrajectoryStatus expectedStatus;
        std::string expectedError;
    };

    std::vector<ValidationTest> tests = {
        // Invalid speeds
        {-1.0, TRACKMAN_DRIVER_LAUNCH, TRACKMAN_DRIVER_SPIN, 
         TrajectoryStatus::InvalidInput, "speed cannot be negative"},
        {150.0, TRACKMAN_DRIVER_LAUNCH, TRACKMAN_DRIVER_SPIN,
         TrajectoryStatus::InvalidInput, "speed exceeds maximum"},
        
        // Invalid angles
        {TRACKMAN_DRIVER_SPEED, -91.0, TRACKMAN_DRIVER_SPIN,
         TrajectoryStatus::InvalidInput, "angle must be between"},
        {TRACKMAN_DRIVER_SPEED, 91.0, TRACKMAN_DRIVER_SPIN,
         TrajectoryStatus::InvalidInput, "angle must be between"},
        
        // Invalid spin rates
        {TRACKMAN_DRIVER_SPEED, TRACKMAN_DRIVER_LAUNCH, -100.0,
         TrajectoryStatus::InvalidInput, "spin rate cannot be negative"},
        {TRACKMAN_DRIVER_SPEED, TRACKMAN_DRIVER_LAUNCH, 15000.0,
         TrajectoryStatus::InvalidInput, "spin rate exceeds maximum"},
        
        // Valid inputs
        {TRACKMAN_DRIVER_SPEED, TRACKMAN_DRIVER_LAUNCH, TRACKMAN_DRIVER_SPIN,
         TrajectoryStatus::Success, ""}
    };

    for (const auto& test : tests) {
        auto result = calculateTrajectoryWithValidation(
            test.speed, test.angle, test.spin, 0.0, 0.0);
        
        EXPECT_EQ(result.status, test.expectedStatus)
            << "Incorrect status for speed=" << test.speed
            << ", angle=" << test.angle
            << ", spin=" << test.spin;
            
        if (test.expectedStatus != TrajectoryStatus::Success) {
            EXPECT_TRUE(result.errorMessage.find(test.expectedError) != std::string::npos)
                << "Expected error message containing '" << test.expectedError
                << "' but got: '" << result.errorMessage << "'";
            EXPECT_FALSE(result.result.has_value()) << "Result should be empty for invalid input";
        } else {
            EXPECT_TRUE(result.result.has_value()) << "Result missing for valid input";
            EXPECT_TRUE(result.errorMessage.empty()) << "Error message present for valid input";
        }
    }
}

// Test physical constraints validation
TEST_F(AccuracyTest, PhysicalConstraintsValidation) {
    // Test extreme conditions that should trigger physical constraint violations
    struct PhysicsTest {
        double speed;
        double angle;
        double spin;
        double wind;
        TrajectoryStatus expectedStatus;
        std::string expectedError;
    };

    std::vector<PhysicsTest> tests = {
        // Extreme speed causing non-physical forces
        {100.0, 45.0, 10000.0, 30.0,
         TrajectoryStatus::CalculationError, "force"},
         
        // Near-vertical trajectory causing convergence issues
        {TRACKMAN_DRIVER_SPEED, 89.0, TRACKMAN_DRIVER_SPIN, 0.0,
         TrajectoryStatus::ConvergenceFailure, "convergence"},
         
        // Valid physics
        {TRACKMAN_DRIVER_SPEED, TRACKMAN_DRIVER_LAUNCH, TRACKMAN_DRIVER_SPIN, 0.0,
         TrajectoryStatus::Success, ""}
    };

    for (const auto& test : tests) {
        auto result = calculateTrajectoryWithValidation(
            test.speed, test.angle, test.spin, test.wind, 0.0);
        
        EXPECT_EQ(result.status, test.expectedStatus)
            << "Incorrect status for extreme physics test";
            
        if (test.expectedStatus != TrajectoryStatus::Success) {
            EXPECT_TRUE(result.errorMessage.find(test.expectedError) != std::string::npos)
                << "Expected error message containing '" << test.expectedError
                << "' but got: '" << result.errorMessage << "'";
        }
    }
}

// Test TrackMan 2024 baseline accuracy
TEST_F(AccuracyTest, TrackMan2024Baseline) {
    // Test baseline driver shot with various conditions
    std::vector<std::pair<double, double>> conditions = {
        {0.0, 20.0},    // Sea level, 20°C
        {1000.0, 15.0}, // 1000m altitude, 15°C
        {2000.0, 10.0}  // 2000m altitude, 10°C
    };

    for (const auto& condition : conditions) {
        double altitude = condition.first;
        double temperature = condition.second;

        // Calculate air density and Reynolds number
        double density = getAirDensity(nullptr, altitude);
        double re = calculateReynoldsNumber(TRACKMAN_DRIVER_SPEED, altitude);
        double cd = calculateDragCoefficient(re);

        TrajectoryResult result = calculateTrajectory(
            TRACKMAN_DRIVER_SPEED,
            TRACKMAN_DRIVER_LAUNCH,
            TRACKMAN_DRIVER_SPIN,
            0.0, 0.0
        );

        // Convert meters to yards for comparison
        double carryYards = result.distance * 1.09361;

        // Verify reasonable results for conditions
        EXPECT_GT(carryYards, 250.0) << "Carry too short at altitude " << altitude << "m";
        EXPECT_LT(carryYards, 300.0) << "Carry too long at altitude " << altitude << "m";
        EXPECT_GT(result.apex, 25.0) << "Apex too low at altitude " << altitude << "m";
        EXPECT_LT(result.apex, 50.0) << "Apex too high at altitude " << altitude << "m";
    }
}

// Test carry distances against reference data
TEST_F(AccuracyTest, CarryDistanceAccuracy) {
    for (const auto& shot : referenceShots) {
        // Calculate trajectory with no wind
        TrajectoryResult result = calculateTrajectory(
            shot.clubSpeed,
            shot.launchAngle,
            shot.spinRate,
            0.0,  // No wind
            0.0   // No wind angle
        );

        // Calculate Reynolds number and drag coefficient
        double re = calculateReynoldsNumber(shot.clubSpeed, 0.0);
        double cd = calculateDragCoefficient(re);

        double carryError = calculateError(shot.expectedCarry, result.distance);
        
        EXPECT_LE(carryError, MAX_CARRY_ERROR_PERCENT)
            << "Carry distance error too high for " << shot.clubType
            << "\nExpected: " << shot.expectedCarry
            << "\nActual: " << result.distance
            << "\nError: " << carryError << "%"
            << "\nReynolds: " << re
            << "\nDrag Coef: " << cd;
    }
}

// Test apex heights against reference data
TEST_F(AccuracyTest, ApexHeightAccuracy) {
    for (const auto& shot : referenceShots) {
        TrajectoryResult result = calculateTrajectory(
            shot.clubSpeed,
            shot.launchAngle,
            shot.spinRate,
            0.0,  // No wind
            0.0   // No wind angle
        );

        // Calculate Magnus force at apex
        double apexVelocity = shot.clubSpeed * 0.7;  // Approximate velocity at apex
        double magnusForce = calculateMagnusForce(shot.spinRate, apexVelocity, BALL_RADIUS);

        double apexError = calculateError(shot.expectedApex, result.apex);
        
        EXPECT_LE(apexError, MAX_APEX_ERROR_PERCENT)
            << "Apex height error too high for " << shot.clubType
            << "\nExpected: " << shot.expectedApex
            << "\nActual: " << result.apex
            << "\nError: " << apexError << "%"
            << "\nMagnus force at apex: " << magnusForce;
    }
}

// Test wind effects accuracy
TEST_F(AccuracyTest, WindEffectsAccuracy) {
    // Test baseline driver shot with various wind conditions
    struct WindTest {
        double speed;
        double angle;
        double expectedCarryChange;  // Percentage change from no-wind carry
    };

    std::vector<WindTest> windTests = {
        {5.0, 0.0, 5.0},     // 5 m/s tailwind should increase carry ~5%
        {5.0, 180.0, -5.0},  // 5 m/s headwind should decrease carry ~5%
        {5.0, 90.0, -2.0},   // 5 m/s crosswind should decrease carry slightly
    };

    // Get baseline carry with no wind
    TrajectoryResult baselineResult = calculateTrajectory(
        TRACKMAN_DRIVER_SPEED,
        TRACKMAN_DRIVER_LAUNCH,
        TRACKMAN_DRIVER_SPIN,
        0.0, 0.0
    );

    for (const auto& test : windTests) {
        // Test with wind gradient
        TrajectoryResult windResult = calculateTrajectory(
            TRACKMAN_DRIVER_SPEED,
            TRACKMAN_DRIVER_LAUNCH,
            TRACKMAN_DRIVER_SPIN,
            test.speed,
            test.angle
        );

        double carryChange = (windResult.distance - baselineResult.distance) / 
                           baselineResult.distance * 100.0;

        EXPECT_NEAR(carryChange, test.expectedCarryChange, 2.0)
            << "Wind effect incorrect for " << test.speed << " m/s at " 
            << test.angle << " degrees";
    }
}

// Test altitude effects accuracy
TEST_F(AccuracyTest, AltitudeEffectsAccuracy) {
    // Test shots at different altitudes
    std::vector<double> altitudes = {0.0, 1000.0, 2000.0};
    std::vector<TrajectoryResult> results;

    for (double altitude : altitudes) {
        double density = getAirDensity(nullptr, altitude);
        double re = calculateReynoldsNumber(TRACKMAN_DRIVER_SPEED, altitude);
        
        TrajectoryResult result = calculateTrajectory(
            TRACKMAN_DRIVER_SPEED,
            TRACKMAN_DRIVER_LAUNCH,
            TRACKMAN_DRIVER_SPIN,
            0.0, 0.0
        );
        results.push_back(result);

        // Verify reasonable results for altitude
        // Typically expect ~2% increase in carry per 1000m elevation
        if (altitude > 0.0) {
            double expectedIncrease = 1.0 + (altitude / 1000.0) * 0.02;
            double actualIncrease = result.distance / results[0].distance;
            EXPECT_NEAR(actualIncrease, expectedIncrease, 0.01)
                << "Incorrect altitude effect at " << altitude << "m";
        }
    }
}
