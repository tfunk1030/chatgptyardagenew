#include <gtest/gtest.h>
#include "data/baseline_data.h"
#include <cmath>

class BaselineDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Nothing needed here as BaselineData initializes itself
    }
    
    // Helper function to verify shot data is within reasonable ranges
    void validateShotData(const BaselineShotData& data, ClubType club) {
        // Basic sanity checks
        EXPECT_GT(data.clubSpeed, 0.0);
        EXPECT_GT(data.ballSpeed, 0.0);
        EXPECT_GT(data.launchAngle, 0.0);
        EXPECT_GT(data.spinRate, 0.0);
        EXPECT_GT(data.carryDistance, 0.0);
        EXPECT_GT(data.totalDistance, data.carryDistance);
        EXPECT_GT(data.maxHeight, 0.0);
        EXPECT_GT(data.landingAngle, 0.0);
        
        // Ball speed should be ~1.44-1.5x club speed for modern clubs
        double smashFactor = data.ballSpeed / data.clubSpeed;
        EXPECT_GE(smashFactor, 1.44);
        EXPECT_LE(smashFactor, 1.50);
        
        // Check club-specific ranges
        if (club == ClubType::DRIVER) {
            EXPECT_GE(data.clubSpeed, 35.0);  // ~78 mph minimum
            EXPECT_LE(data.clubSpeed, 55.0);  // ~123 mph maximum
            EXPECT_GE(data.launchAngle, 8.0);
            EXPECT_LE(data.launchAngle, 15.0);
            EXPECT_GE(data.spinRate, 2000);
            EXPECT_LE(data.spinRate, 3500);
        } else if (club == ClubType::SEVEN_IRON) {
            EXPECT_GE(data.clubSpeed, 25.0);  // ~56 mph minimum
            EXPECT_LE(data.clubSpeed, 40.0);  // ~89 mph maximum
            EXPECT_GE(data.launchAngle, 15.0);
            EXPECT_LE(data.launchAngle, 22.0);
            EXPECT_GE(data.spinRate, 6000);
            EXPECT_LE(data.spinRate, 8000);
        }
    }
};

// Test baseline data for different skill levels with driver
TEST_F(BaselineDataTest, DriverSkillLevels) {
    ClubType club = ClubType::DRIVER;
    
    // Get data for each skill level
    auto tourData = BaselineData::getBaseline(club, SkillLevel::TOUR);
    auto scratchData = BaselineData::getBaseline(club, SkillLevel::SCRATCH);
    auto lowData = BaselineData::getBaseline(club, SkillLevel::LOW_HANDICAP);
    auto midData = BaselineData::getBaseline(club, SkillLevel::MID_HANDICAP);
    auto highData = BaselineData::getBaseline(club, SkillLevel::HIGH_HANDICAP);
    
    // Validate each dataset
    validateShotData(tourData, club);
    validateShotData(scratchData, club);
    validateShotData(lowData, club);
    validateShotData(midData, club);
    validateShotData(highData, club);
    
    // Verify skill level progression
    EXPECT_GT(tourData.clubSpeed, scratchData.clubSpeed);
    EXPECT_GT(scratchData.clubSpeed, lowData.clubSpeed);
    EXPECT_GT(lowData.clubSpeed, midData.clubSpeed);
    EXPECT_GT(midData.clubSpeed, highData.clubSpeed);
    
    // Verify carry distances follow the same pattern
    EXPECT_GT(tourData.carryDistance, scratchData.carryDistance);
    EXPECT_GT(scratchData.carryDistance, lowData.carryDistance);
    EXPECT_GT(lowData.carryDistance, midData.carryDistance);
    EXPECT_GT(midData.carryDistance, highData.carryDistance);
}

// Test baseline data for different skill levels with 7-iron
TEST_F(BaselineDataTest, SevenIronSkillLevels) {
    ClubType club = ClubType::SEVEN_IRON;
    
    // Get data for each skill level
    auto tourData = BaselineData::getBaseline(club, SkillLevel::TOUR);
    auto scratchData = BaselineData::getBaseline(club, SkillLevel::SCRATCH);
    auto lowData = BaselineData::getBaseline(club, SkillLevel::LOW_HANDICAP);
    auto midData = BaselineData::getBaseline(club, SkillLevel::MID_HANDICAP);
    auto highData = BaselineData::getBaseline(club, SkillLevel::HIGH_HANDICAP);
    
    // Validate each dataset
    validateShotData(tourData, club);
    validateShotData(scratchData, club);
    validateShotData(lowData, club);
    validateShotData(midData, club);
    validateShotData(highData, club);
    
    // Verify spin rates are higher than driver
    auto tourDriver = BaselineData::getBaseline(ClubType::DRIVER, SkillLevel::TOUR);
    EXPECT_GT(tourData.spinRate, tourDriver.spinRate);
}

// Test variation ranges
TEST_F(BaselineDataTest, VariationRanges) {
    // Get variation ranges for each skill level
    auto tourRange = BaselineData::getVariationRange(SkillLevel::TOUR);
    auto scratchRange = BaselineData::getVariationRange(SkillLevel::SCRATCH);
    auto lowRange = BaselineData::getVariationRange(SkillLevel::LOW_HANDICAP);
    auto midRange = BaselineData::getVariationRange(SkillLevel::MID_HANDICAP);
    auto highRange = BaselineData::getVariationRange(SkillLevel::HIGH_HANDICAP);
    
    // Verify progression of variations (higher handicap = more variation)
    EXPECT_LT(tourRange.speedVariation, scratchRange.speedVariation);
    EXPECT_LT(scratchRange.speedVariation, lowRange.speedVariation);
    EXPECT_LT(lowRange.speedVariation, midRange.speedVariation);
    EXPECT_LT(midRange.speedVariation, highRange.speedVariation);
    
    // Verify angle variations follow same pattern
    EXPECT_LT(tourRange.angleVariation, scratchRange.angleVariation);
    EXPECT_LT(scratchRange.angleVariation, lowRange.angleVariation);
    EXPECT_LT(lowRange.angleVariation, midRange.angleVariation);
    EXPECT_LT(midRange.angleVariation, highRange.angleVariation);
}

// Test club specifications
TEST_F(BaselineDataTest, ClubSpecs) {
    // Test loft angles
    EXPECT_NEAR(BaselineData::getClubLoft(ClubType::DRIVER), 10.5, 0.1);
    EXPECT_NEAR(BaselineData::getClubLoft(ClubType::SEVEN_IRON), 31.0, 0.1);
    EXPECT_NEAR(BaselineData::getClubLoft(ClubType::PITCHING_WEDGE), 45.0, 0.1);
    
    // Test club lengths
    EXPECT_NEAR(BaselineData::getClubLength(ClubType::DRIVER), 1.143, 0.001);  // 45"
    EXPECT_NEAR(BaselineData::getClubLength(ClubType::SEVEN_IRON), 0.953, 0.001);  // 37.5"
    EXPECT_NEAR(BaselineData::getClubLength(ClubType::PITCHING_WEDGE), 0.914, 0.001);  // 36"
}

// Test string conversions
TEST_F(BaselineDataTest, StringConversions) {
    // Test club type to string
    EXPECT_EQ(BaselineData::clubTypeToString(ClubType::DRIVER), "Driver");
    EXPECT_EQ(BaselineData::clubTypeToString(ClubType::SEVEN_IRON), "7 Iron");
    EXPECT_EQ(BaselineData::clubTypeToString(ClubType::PITCHING_WEDGE), "PW");
    
    // Test string to club type
    EXPECT_EQ(BaselineData::stringToClubType("Driver"), ClubType::DRIVER);
    EXPECT_EQ(BaselineData::stringToClubType("7 Iron"), ClubType::SEVEN_IRON);
    EXPECT_EQ(BaselineData::stringToClubType("PW"), ClubType::PITCHING_WEDGE);
    
    // Test invalid string conversion
    EXPECT_THROW(BaselineData::stringToClubType("Invalid Club"), std::runtime_error);
}
