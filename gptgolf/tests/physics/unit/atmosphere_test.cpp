#include <gtest/gtest.h>
#include "physics/atmosphere.h"
#include <cmath>

class AtmosphereTest : public ::testing::Test {
protected:
    const double TOLERANCE = 1e-4;
    AtmosphericModel atmosphere;
};

// Test temperature calculations at different altitudes
TEST_F(AtmosphereTest, TemperatureProfile) {
    // Sea level temperature should match ISA standard
    EXPECT_NEAR(atmosphere.getTemperature(0), 288.15, TOLERANCE);
    
    // Test temperature at tropopause (11km)
    EXPECT_NEAR(atmosphere.getTemperature(11000), 216.65, TOLERANCE);
    
    // Test temperature in stratosphere
    EXPECT_NEAR(atmosphere.getTemperature(25000), 221.65, TOLERANCE);
    
    // Verify temperature gradient in troposphere
    double temp1 = atmosphere.getTemperature(0);
    double temp2 = atmosphere.getTemperature(1000);
    EXPECT_NEAR(temp1 - temp2, 6.5, TOLERANCE); // Should decrease by 6.5K per 1000m
}

// Test pressure calculations
TEST_F(AtmosphereTest, PressureProfile) {
    // Sea level pressure should match ISA standard
    EXPECT_NEAR(atmosphere.getPressure(0), 101325.0, 1.0);
    
    // Test pressure at various altitudes
    struct TestPoint {
        double altitude;
        double expectedPressure;
    };
    
    std::vector<TestPoint> testPoints = {
        {5000, 54019.9},    // Mid-troposphere
        {11000, 22632.1},   // Tropopause
        {20000, 5474.89},   // Lower stratosphere
    };
    
    for (const auto& point : testPoints) {
        double pressure = atmosphere.getPressure(point.altitude);
        EXPECT_NEAR(pressure, point.expectedPressure, point.expectedPressure * 0.01)
            << "Pressure mismatch at altitude " << point.altitude;
    }
    
    // Verify pressure decreases with altitude
    for (double h = 0; h < 40000; h += 1000) {
        EXPECT_GT(atmosphere.getPressure(h), atmosphere.getPressure(h + 1000))
            << "Pressure inversion detected at altitude " << h;
    }
}

// Test density calculations
TEST_F(AtmosphereTest, DensityProfile) {
    // Test standard atmosphere density at sea level
    EXPECT_NEAR(atmosphere.getDensity(0, nullptr), 1.225, 0.001);
    
    // Test density decrease with altitude
    double rho0 = atmosphere.getDensity(0, nullptr);
    double rho5k = atmosphere.getDensity(5000, nullptr);
    double rho10k = atmosphere.getDensity(10000, nullptr);
    
    EXPECT_GT(rho0, rho5k);
    EXPECT_GT(rho5k, rho10k);
    
    // Verify density ratio at typical golf shot altitudes
    double densityRatio100m = atmosphere.getDensity(100, nullptr) / atmosphere.getDensity(0, nullptr);
    EXPECT_NEAR(densityRatio100m, 0.988, 0.001);
}

// Test weather data integration
TEST_F(AtmosphereTest, WeatherEffects) {
    WeatherData weather;
    weather.temperature = 30.0;  // Hot day (30°C)
    weather.pressure = 1013.25;  // Standard pressure (hPa)
    weather.humidity = 80.0;     // High humidity
    
    // Get density with and without weather data
    double standardDensity = atmosphere.getDensity(0, nullptr);
    double actualDensity = atmosphere.getDensity(0, &weather);
    
    // Hot air should be less dense
    EXPECT_LT(actualDensity, standardDensity);
    
    // Test humidity effect
    weather.humidity = 0.0;  // Dry air
    double dryDensity = atmosphere.getDensity(0, &weather);
    EXPECT_GT(dryDensity, actualDensity);  // Humid air is less dense
}

// Test layer transitions
TEST_F(AtmosphereTest, LayerTransitions) {
    // Test continuity at layer boundaries
    std::vector<double> layerBoundaries = {11000, 20000, 32000, 47000};
    
    for (double altitude : layerBoundaries) {
        double tempBelow = atmosphere.getTemperature(altitude - 1);
        double tempAbove = atmosphere.getTemperature(altitude + 1);
        double pressBelow = atmosphere.getPressure(altitude - 1);
        double pressAbove = atmosphere.getPressure(altitude + 1);
        
        // Temperature and pressure should be continuous
        EXPECT_NEAR(tempBelow, tempAbove, 0.1);
        EXPECT_NEAR(pressBelow, pressAbove, pressBelow * 0.01);
    }
}
