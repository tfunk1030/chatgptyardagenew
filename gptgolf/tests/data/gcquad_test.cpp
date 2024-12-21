#include <gtest/gtest.h>
#include "data/launch_monitor.h"
#include "data/gcquad_monitor.h"
#include <thread>
#include <chrono>

using namespace gptgolf::data;

class GCQuadTest : public ::testing::Test {
protected:
    void SetUp() override {
        monitor = std::make_unique<GCQuadMonitor>();
    }

    void TearDown() override {
        if (monitor->isConnected()) {
            monitor->disconnect();
        }
    }

    std::unique_ptr<GCQuadMonitor> monitor;
};

TEST_F(GCQuadTest, CameraConfigurationTest) {
    EXPECT_TRUE(monitor->connect());
    
    // Test quadruplex mode
    EXPECT_TRUE(monitor->configure("quadruplex", "true"));
    EXPECT_EQ(monitor->getSetting("quadruplex"), "true");
    
    // Test dual camera mode
    EXPECT_TRUE(monitor->configure("quadruplex", "false"));
    EXPECT_EQ(monitor->getSetting("quadruplex"), "false");
    
    // Verify data quality difference between modes
    monitor->configure("quadruplex", "true");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto quadData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    monitor->configure("quadruplex", "false");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto dualData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (quadData && dualData) {
        EXPECT_GT(quadData->confidence, dualData->confidence);
    }
}

TEST_F(GCQuadTest, BallModelTest) {
    EXPECT_TRUE(monitor->connect());
    
    // Test valid ball models
    EXPECT_TRUE(monitor->configure("ballModel", "ProV1"));
    EXPECT_TRUE(monitor->configure("ballModel", "ProV1x"));
    EXPECT_TRUE(monitor->configure("ballModel", "TP5"));
    EXPECT_TRUE(monitor->configure("ballModel", "Generic"));
    
    // Test invalid ball model
    EXPECT_FALSE(monitor->configure("ballModel", "InvalidBall"));
    
    // Test ball model effects on data
    monitor->configure("ballModel", "ProV1");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto prov1Data = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    monitor->configure("ballModel", "Generic");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto genericData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (prov1Data && genericData) {
        // ProV1 should generate more spin
        EXPECT_GT(prov1Data->spinRate, genericData->spinRate);
    }
}

TEST_F(GCQuadTest, HighSpeedModeTest) {
    EXPECT_TRUE(monitor->connect());
    
    // Test high speed mode configuration
    EXPECT_TRUE(monitor->configure("highSpeed", "true"));
    EXPECT_EQ(monitor->getSetting("highSpeed"), "true");
    
    // Test normal speed mode
    EXPECT_TRUE(monitor->configure("highSpeed", "false"));
    EXPECT_EQ(monitor->getSetting("highSpeed"), "false");
    
    // Verify capture rate differences
    monitor->configure("highSpeed", "true");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto highSpeedData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    monitor->configure("highSpeed", "false");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto normalSpeedData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (highSpeedData && normalSpeedData) {
        // High speed mode should have higher confidence
        EXPECT_GT(highSpeedData->confidence, normalSpeedData->confidence);
    }
}

TEST_F(GCQuadTest, DataQualityTest) {
    EXPECT_TRUE(monitor->connect());
    EXPECT_TRUE(monitor->startTracking());
    
    // Configure for best quality
    monitor->configure("quadruplex", "true");
    monitor->configure("highSpeed", "true");
    monitor->configure("environment", "Indoor");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto data = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (data) {
        // GCQuad should provide high-quality data
        EXPECT_GT(data->confidence, 0.95);
        EXPECT_GT(data->ballSpeed, 0);
        EXPECT_LT(data->ballSpeed, 100.0);  // Max ~220mph
        EXPECT_GE(data->launchAngle, -10.0);
        EXPECT_LE(data->launchAngle, 60.0);
        
        // Verify spin measurements
        EXPECT_GE(data->spinRate, 0);
        EXPECT_LE(data->spinRate, 12000.0);
        
        // Club data validation
        EXPECT_GT(data->clubSpeed, 0);
        EXPECT_LT(data->clubSpeed, 67.0);  // Max ~150mph
        EXPECT_GE(data->smashFactor, 1.0);
        EXPECT_LE(data->smashFactor, 1.5);
    }
}

TEST_F(GCQuadTest, EnvironmentAdjustmentTest) {
    EXPECT_TRUE(monitor->connect());
    
    // Test indoor environment
    monitor->configure("environment", "Indoor");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto indoorData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    // Test outdoor environment
    monitor->configure("environment", "Outdoor");
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto outdoorData = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (indoorData && outdoorData) {
        // Indoor data should have higher confidence
        EXPECT_GT(indoorData->confidence, outdoorData->confidence);
        
        // Verify environmental adjustments
        EXPECT_NE(indoorData->ballSpeed, outdoorData->ballSpeed);
        EXPECT_NE(indoorData->spinRate, outdoorData->spinRate);
    }
}

TEST_F(GCQuadTest, CalibrationTest) {
    // Test initial connection with calibration
    EXPECT_TRUE(monitor->connect());
    
    // Test tracking after calibration
    EXPECT_TRUE(monitor->startTracking());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto data = monitor->getLastShot();
    EXPECT_TRUE(monitor->stopTracking());
    
    if (data) {
        // Calibrated data should have high confidence
        EXPECT_GT(data->confidence, 0.9);
    }
}
