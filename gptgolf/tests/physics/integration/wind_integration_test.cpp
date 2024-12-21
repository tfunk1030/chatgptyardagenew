#include <gtest/gtest.h>
#include "physics/wind.h"
#include <cmath>

class WindIntegrationTest : public ::testing::Test {
protected:
    static constexpr double EPSILON = 1e-6;
    
    // Helper function to calculate frontend-style wind effect
    double calculateFrontendWindEffect(double speed, double direction, 
                                     const std::string& shotType = "normal",
                                     const std::string& terrain = "fairway") {
        // Convert direction to radians
        double windRad = direction * M_PI / 180.0;
        double headwind = -speed * cos(windRad);
        double crosswind = speed * sin(windRad);
        
        // Shot type multipliers (matching frontend)
        double shotTypeMultiplier = 1.0;
        if (shotType == "low") shotTypeMultiplier = 0.7;
        if (shotType == "high") shotTypeMultiplier = 1.4;
        
        // Terrain factors (matching frontend)
        double terrainFactor = 1.0;
        if (terrain == "water") terrainFactor = 1.2;
        if (terrain == "rough") terrainFactor = 0.8;
        
        // Height multiplier (matching frontend)
        double heightMultiplier = 1.0;
        if (shotType == "low") heightMultiplier = 0.8;
        if (shotType == "high") heightMultiplier = 1.3;
        
        // Calculate effects
        double headwindCoeff = 0.35 * shotTypeMultiplier * terrainFactor;
        double crosswindCoeff = 0.15 * heightMultiplier * terrainFactor;
        
        double headwindEffect = headwind * headwindCoeff;
        double crosswindEffect = -fabs(crosswind) * crosswindCoeff;
        
        // Strong wind scaling
        double windStrengthMultiplier = speed > 15 ? 1 + (speed - 15) * 0.02 : 1;
        return (headwindEffect + crosswindEffect) * windStrengthMultiplier;
    }
};

TEST_F(WindIntegrationTest, CompareHeadwindEffects) {
    // Setup physics engine wind
    Wind wind(10.0, 0.0, WindProfile::CONSTANT); // 10 mph headwind
    Point3D position(0.0, 0.0, 30.0); // Standard shot height
    double ballVelocity = 150.0; // ~100 mph ball speed
    
    // Get physics engine result
    Point3D result = wind.applyWindEffect(position, ballVelocity);
    double physicsEffect = (result.x - position.x) / position.x * 100.0; // Convert to percentage
    
    // Get frontend calculation
    double frontendEffect = calculateFrontendWindEffect(10.0, 0.0);
    
    // Compare results (allowing for small differences due to different models)
    EXPECT_NEAR(frontendEffect, physicsEffect, 1.0); // Within 1% difference
}

TEST_F(WindIntegrationTest, CompareCrosswindEffects) {
    Wind wind(10.0, 90.0, WindProfile::CONSTANT); // 10 mph crosswind
    Point3D position(0.0, 0.0, 30.0);
    double ballVelocity = 150.0;
    
    Point3D result = wind.applyWindEffect(position, ballVelocity);
    double physicsEffect = (result.y - position.y) / position.y * 100.0;
    
    double frontendEffect = calculateFrontendWindEffect(10.0, 90.0);
    
    EXPECT_NEAR(frontendEffect, physicsEffect, 1.0);
}

TEST_F(WindIntegrationTest, CompareTerrainEffects) {
    // Test water terrain
    TerrainParameters waterTerrain = TerrainParameters::Water();
    Wind waterWind(10.0, 0.0, WindProfile::CONSTANT, waterTerrain);
    
    Point3D position(0.0, 0.0, 30.0);
    double ballVelocity = 150.0;
    
    Point3D waterResult = wind.applyWindEffect(position, ballVelocity);
    double physicsWaterEffect = (waterResult.x - position.x) / position.x * 100.0;
    
    double frontendWaterEffect = calculateFrontendWindEffect(10.0, 0.0, "normal", "water");
    
    EXPECT_NEAR(frontendWaterEffect, physicsWaterEffect, 1.0);
}

TEST_F(WindIntegrationTest, CompareShotTypeEffects) {
    Wind wind(10.0, 0.0, WindProfile::CONSTANT);
    Point3D lowPosition(0.0, 0.0, 15.0);  // Lower trajectory
    Point3D highPosition(0.0, 0.0, 45.0); // Higher trajectory
    double ballVelocity = 150.0;
    
    // Compare low shots
    Point3D lowResult = wind.applyWindEffect(lowPosition, ballVelocity);
    double physicsLowEffect = (lowResult.x - lowPosition.x) / lowPosition.x * 100.0;
    double frontendLowEffect = calculateFrontendWindEffect(10.0, 0.0, "low");
    EXPECT_NEAR(frontendLowEffect, physicsLowEffect, 1.0);
    
    // Compare high shots
    Point3D highResult = wind.applyWindEffect(highPosition, ballVelocity);
    double physicsHighEffect = (highResult.x - highPosition.x) / highPosition.x * 100.0;
    double frontendHighEffect = calculateFrontendWindEffect(10.0, 0.0, "high");
    EXPECT_NEAR(frontendHighEffect, physicsHighEffect, 1.0);
}

TEST_F(WindIntegrationTest, CompareStrongWindScaling) {
    // Test strong wind (20 mph)
    Wind strongWind(20.0, 0.0, WindProfile::CONSTANT);
    Point3D position(0.0, 0.0, 30.0);
    double ballVelocity = 150.0;
    
    Point3D result = strongWind.applyWindEffect(position, ballVelocity);
    double physicsEffect = (result.x - position.x) / position.x * 100.0;
    
    double frontendEffect = calculateFrontendWindEffect(20.0, 0.0);
    
    // Verify non-linear scaling matches between implementations
    EXPECT_NEAR(frontendEffect, physicsEffect, 1.5); // Allow slightly larger difference for strong winds
    EXPECT_GT(abs(frontendEffect), abs(calculateFrontendWindEffect(10.0, 0.0)) * 1.8); // Should be more than double
}

TEST_F(WindIntegrationTest, CompareComplexConditions) {
    // Test complex scenario with diagonal wind, rough terrain, and high shot
    TerrainParameters roughTerrain = TerrainParameters::Urban(); // Similar to rough
    Wind wind(15.0, 45.0, WindProfile::CONSTANT, roughTerrain);
    Point3D position(0.0, 0.0, 45.0); // High shot
    double ballVelocity = 150.0;
    
    Point3D result = wind.applyWindEffect(position, ballVelocity);
    
    // Calculate total displacement effect
    double dx = result.x - position.x;
    double dy = result.y - position.y;
    double totalDisplacement = sqrt(dx*dx + dy*dy);
    double physicsEffect = totalDisplacement / sqrt(position.x*position.x + position.y*position.y) * 100.0;
    
    double frontendEffect = calculateFrontendWindEffect(15.0, 45.0, "high", "rough");
    
    EXPECT_NEAR(frontendEffect, physicsEffect, 1.5);
}
