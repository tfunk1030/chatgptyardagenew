#include <gtest/gtest.h>
#include "data/launch_monitor.h"
#include "data/trackman_monitor.h"
#include <thread>
#include <chrono>

using namespace gptgolf::data;

class LaunchMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        monitor = std::make_unique<TrackManMonitor>();
    }

    void TearDown() override {
        if (monitor->isConnected()) {
            monitor->disconnect();
        }
    }

    std::unique_ptr<ILaunchMonitor> monitor;
};

TEST_F(LaunchMonitorTest, ConnectionTest) {
    EXPECT_FALSE(monitor->isConnected());
    EXPECT_TRUE(monitor->connect());
    EXPECT_TRUE(monitor->isConnected());
    EXPECT_TRUE(monitor->disconnect());
    EXPECT_FALSE(monitor->isConnected());
}

TEST_F(LaunchMonitorTest, TrackingTest) {
    EXPECT_TRUE(monitor->connect());
    EXPECT_FALSE(monitor->isTracking());
    EXPECT_TRUE(monitor->startTracking());
    EXPECT_TRUE(monitor->isTracking());
    
    // Wait for some data collection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(monitor->stopTracking());
    EXPECT_FALSE(monitor->isTracking());
}

TEST_F(LaunchMonitorTest, DataValidationTest) {
    EXPECT_TRUE(monitor->connect());
    EXPECT_TRUE(monitor->startTracking());
    
    // Wait for data collection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto data = monitor->getLastShot();
    EXPECT_TRUE(data.has_value());
    
    if (data) {
        // Validate ball data
        EXPECT_GT(data->ballSpeed, 0);
        EXPECT_LT(data->ballSpeed, 100.0);  // Max ~220mph
        EXPECT_GE(data->launchAngle, -10.0);
        EXPECT_LE(data->launchAngle, 60.0);
        EXPECT_GE(data->spinRate, 0);
        EXPECT_LE(data->spinRate, 12000.0);
        
        // Validate club data
        EXPECT_GT(data->clubSpeed, 0);
        EXPECT_LT(data->clubSpeed, 67.0);  // Max ~150mph
        EXPECT_GE(data->smashFactor, 1.0);
        EXPECT_LE(data->smashFactor, 1.5);
    }
    
    EXPECT_TRUE(monitor->stopTracking());
}

TEST_F(LaunchMonitorTest, ConfigurationTest) {
    // Test valid configurations
    EXPECT_TRUE(monitor->configure("units", "Metric"));
    EXPECT_TRUE(monitor->configure("units", "Imperial"));
    EXPECT_TRUE(monitor->configure("environment", "Indoor"));
    EXPECT_TRUE(monitor->configure("environment", "Outdoor"));
    EXPECT_TRUE(monitor->configure("normalize", "true"));
    
    // Test invalid configurations
    EXPECT_FALSE(monitor->configure("units", "Invalid"));
    EXPECT_FALSE(monitor->configure("environment", "Invalid"));
    EXPECT_FALSE(monitor->configure("invalid", "value"));
    
    // Verify settings
    EXPECT_EQ(monitor->getSetting("units"), "Imperial");
    EXPECT_EQ(monitor->getSetting("environment"), "Outdoor");
    EXPECT_EQ(monitor->getSetting("normalize"), "true");
}

TEST_F(LaunchMonitorTest, DataConversionTest) {
    EXPECT_TRUE(monitor->connect());
    EXPECT_TRUE(monitor->startTracking());
    
    // Wait for data collection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto launchData = monitor->getLastShot();
    EXPECT_TRUE(launchData.has_value());
    
    if (launchData) {
        // Convert to ShotData
        ShotData shotData = monitor->convertToShotData(*launchData);
        
        // Verify conversion
        EXPECT_EQ(shotData.initialVelocity, launchData->ballSpeed);
        EXPECT_EQ(shotData.spinRate, launchData->spinRate);
        EXPECT_EQ(shotData.launchAngle, launchData->launchAngle);
        EXPECT_EQ(shotData.actualDistance, launchData->carryDistance);
        EXPECT_EQ(shotData.lateralDeviation, launchData->ballHorizontal);
    }
    
    EXPECT_TRUE(monitor->stopTracking());
}

TEST_F(LaunchMonitorTest, FactoryTest) {
    // Test factory creation
    auto devices = LaunchMonitorFactory::getSupportedDevices();
    EXPECT_FALSE(devices.empty());
    EXPECT_TRUE(std::find(devices.begin(), devices.end(), "TrackMan") != devices.end());
    
    // Test creating TrackMan monitor
    auto trackman = LaunchMonitorFactory::create("TrackMan");
    EXPECT_NE(trackman, nullptr);
    
    // Test invalid device type
    EXPECT_THROW(LaunchMonitorFactory::create("InvalidDevice"), std::runtime_error);
}

TEST_F(LaunchMonitorTest, EnvironmentAdjustmentTest) {
    EXPECT_TRUE(monitor->connect());
    
    // Test indoor environment
    EXPECT_TRUE(monitor->configure("environment", "Indoor"));
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto indoorData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    // Test outdoor environment
    EXPECT_TRUE(monitor->configure("environment", "Outdoor"));
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto outdoorData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (indoorData && outdoorData) {
        // Indoor data should have higher confidence due to controlled environment
        EXPECT_GT(indoorData->confidence, outdoorData->confidence);
    }
}

TEST_F(LaunchMonitorTest, NormalizationTest) {
    EXPECT_TRUE(monitor->connect());
    
    // Test with normalization enabled
    EXPECT_TRUE(monitor->configure("normalize", "true"));
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto normalizedData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    // Test with normalization disabled
    EXPECT_TRUE(monitor->configure("normalize", "false"));
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto rawData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (normalizedData && rawData) {
        // Normalized data should be adjusted for conditions
        EXPECT_NE(normalizedData->ballSpeed, rawData->ballSpeed);
        EXPECT_NE(normalizedData->spinRate, rawData->spinRate);
    }
}
