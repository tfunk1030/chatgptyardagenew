#include <gtest/gtest.h>
#include "physics/wind.h"
#include <cmath>

class WindTest : public ::testing::Test {
protected:
    static constexpr double EPSILON = 1e-6;
    
    // Helper function to compare Points3D
    void expectPointsNear(const Point3D& actual, const Point3D& expected, double epsilon = EPSILON) {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }
};

TEST_F(WindTest, ConstantProfile) {
    Wind wind(10.0, 90.0, WindProfile::CONSTANT);  // 10 m/s from West
    
    // Speed should be constant with height
    EXPECT_DOUBLE_EQ(wind.getSpeedAtHeight(0.0), 10.0);
    EXPECT_DOUBLE_EQ(wind.getSpeedAtHeight(100.0), 10.0);
    EXPECT_DOUBLE_EQ(wind.getSpeedAtHeight(1000.0), 10.0);
    
    // Direction should remain constant
    EXPECT_DOUBLE_EQ(wind.getDirectionAtHeight(0.0), 90.0);
    EXPECT_DOUBLE_EQ(wind.getDirectionAtHeight(100.0), 90.0);
}

TEST_F(WindTest, LogarithmicProfile) {
    TerrainParameters terrain = TerrainParameters::OpenTerrain();
    Wind wind(10.0, 90.0, WindProfile::LOGARITHMIC, terrain);
    
    // Speed should increase logarithmically with height
    double speed10m = wind.getSpeedAtHeight(10.0);  // Reference height
    double speed20m = wind.getSpeedAtHeight(20.0);
    double speed40m = wind.getSpeedAtHeight(40.0);
    
    EXPECT_DOUBLE_EQ(speed10m, 10.0);  // Reference speed at reference height
    EXPECT_GT(speed20m, speed10m);     // Speed increases with height
    EXPECT_GT(speed40m, speed20m);
    
    // Verify logarithmic relationship
    double ratio1 = (speed20m - speed10m) / log(20.0/10.0);
    double ratio2 = (speed40m - speed20m) / log(40.0/20.0);
    EXPECT_NEAR(ratio1, ratio2, 0.1);  // Should follow log law
}

TEST_F(WindTest, PowerLawProfile) {
    TerrainParameters terrain = TerrainParameters::OpenTerrain();
    Wind wind(10.0, 90.0, WindProfile::POWER_LAW, terrain);
    
    // Speed should follow power law with height
    double speed10m = wind.getSpeedAtHeight(10.0);
    double speed20m = wind.getSpeedAtHeight(20.0);
    
    // Verify power law relationship
    double expectedRatio = pow(20.0/10.0, terrain.powerLawExponent);
    double actualRatio = speed20m / speed10m;
    EXPECT_NEAR(actualRatio, expectedRatio, EPSILON);
}

TEST_F(WindTest, EkmanSpiral) {
    Wind wind(10.0, 90.0, WindProfile::EKMAN_SPIRAL);
    
    // Direction should change with height
    double dir100m = wind.getDirectionAtHeight(100.0);
    double dir500m = wind.getDirectionAtHeight(500.0);
    double dir1000m = wind.getDirectionAtHeight(1000.0);
    
    // Verify direction changes clockwise with height (Northern Hemisphere)
    EXPECT_GT(dir500m, dir100m);
    EXPECT_GT(dir1000m, dir500m);
    
    // Direction change should be reasonable
    EXPECT_LT(dir1000m - dir100m, 45.0);  // Typically less than 45° over 1km
}

TEST_F(WindTest, TerrainEffects) {
    double speed = 10.0;
    double direction = 90.0;
    
    Wind waterWind(speed, direction, WindProfile::LOGARITHMIC, TerrainParameters::Water());
    Wind openWind(speed, direction, WindProfile::LOGARITHMIC, TerrainParameters::OpenTerrain());
    Wind urbanWind(speed, direction, WindProfile::LOGARITHMIC, TerrainParameters::Urban());
    
    double height = 50.0;
    
    // Wind speed should be higher over smooth terrain
    EXPECT_GT(waterWind.getSpeedAtHeight(height), openWind.getSpeedAtHeight(height));
    EXPECT_GT(openWind.getSpeedAtHeight(height), urbanWind.getSpeedAtHeight(height));
}

TEST_F(WindTest, WindEffect) {
    Wind wind(10.0, 90.0);  // 10 m/s from West
    Point3D position(0.0, 0.0, 30.0);
    double ballVelocity = 50.0;  // m/s
    
    Point3D result = wind.applyWindEffect(position, ballVelocity);
    
    // Ball should be pushed eastward (positive y) by westerly wind
    EXPECT_GT(result.y, position.y);
    
    // Effect should be proportional to wind speed and inverse to ball velocity
    Wind strongerWind(20.0, 90.0);
    Point3D strongerResult = strongerWind.applyWindEffect(position, ballVelocity);
    EXPECT_GT(strongerResult.y - position.y, result.y - position.y);
    
    // Effect should be stronger at higher altitudes
    Point3D highPosition(0.0, 0.0, 100.0);
    Point3D highResult = wind.applyWindEffect(highPosition, ballVelocity);
    EXPECT_GT(highResult.y - highPosition.y, result.y - position.y);
}

TEST_F(WindTest, RoughTerrainVerticalEffect) {
    TerrainParameters urbanTerrain = TerrainParameters::Urban();
    Wind wind(10.0, 90.0, WindProfile::LOGARITHMIC, urbanTerrain);
    
    Point3D position(0.0, 0.0, 30.0);
    double ballVelocity = 50.0;
    
    Point3D result = wind.applyWindEffect(position, ballVelocity);
    
    // Rough terrain should induce some vertical displacement
    EXPECT_NE(result.z, position.z);
}

TEST_F(WindTest, ProfileTransitions) {
    Wind wind(10.0, 90.0);
    
    // Test switching between profiles
    wind.setProfile(WindProfile::CONSTANT);
    EXPECT_EQ(wind.getProfile(), WindProfile::CONSTANT);
    
    wind.setProfile(WindProfile::LOGARITHMIC);
    EXPECT_EQ(wind.getProfile(), WindProfile::LOGARITHMIC);
    
    // Test switching terrain
    TerrainParameters urban = TerrainParameters::Urban();
    wind.setTerrain(urban);
    EXPECT_EQ(wind.getTerrain().roughnessLength, urban.roughnessLength);
}

TEST_F(WindTest, EdgeCases) {
    Wind wind(10.0, 90.0);
    
    // Test very low heights
    EXPECT_NEAR(wind.getSpeedAtHeight(0.0), 0.0, EPSILON);
    
    // Test very high speeds
    Wind strongWind(100.0, 90.0);
    Point3D position(0.0, 0.0, 30.0);
    double ballVelocity = 50.0;
    
    Point3D result = strongWind.applyWindEffect(position, ballVelocity);
    EXPECT_GT(result.y, position.y);  // Should still behave reasonably
    
    // Test extreme angles
    Wind extremeWind(10.0, 359.0);
    EXPECT_NEAR(extremeWind.getDirectionAtHeight(30.0), 359.0, EPSILON);
}
