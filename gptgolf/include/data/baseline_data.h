#pragma once

#include <vector>
#include <string>
#include <map>

namespace gptgolf {
namespace data {

// Club types
enum class ClubType {
    DRIVER,
    THREE_WOOD,
    FIVE_WOOD,
    FOUR_IRON,
    FIVE_IRON,
    SIX_IRON,
    SEVEN_IRON,
    EIGHT_IRON,
    NINE_IRON,
    PITCHING_WEDGE,
    GAP_WEDGE,
    SAND_WEDGE,
    LOB_WEDGE
};

// Shot data structure
struct BaselineShotData {
    double clubSpeed;        // m/s
    double ballSpeed;        // m/s
    double launchAngle;      // degrees
    double spinRate;         // rpm
    double spinAxis;         // degrees (0 = pure backspin, positive = draw, negative = fade)
    double carryDistance;    // meters
    double totalDistance;    // meters
    double maxHeight;        // meters
    double landingAngle;     // degrees
    
    BaselineShotData(double cs = 0.0, double bs = 0.0, double la = 0.0, 
                    double sr = 0.0, double sa = 0.0, double cd = 0.0,
                    double td = 0.0, double mh = 0.0, double lng = 0.0)
        : clubSpeed(cs), ballSpeed(bs), launchAngle(la), spinRate(sr),
          spinAxis(sa), carryDistance(cd), totalDistance(td),
          maxHeight(mh), landingAngle(lng) {}
};

// Skill levels
enum class SkillLevel {
    TOUR,           // Tour professional
    SCRATCH,        // Scratch golfer (0 handicap)
    LOW_HANDICAP,   // 1-9 handicap
    MID_HANDICAP,   // 10-18 handicap
    HIGH_HANDICAP   // 19+ handicap
};

// Variation range structure
struct VariationRange {
    double speedVariation;      // m/s
    double angleVariation;      // degrees
    double spinVariation;       // rpm
    double spinAxisVariation;   // degrees
};

class BaselineData {
public:
    // Get baseline data for a specific club and skill level
    static BaselineShotData getBaseline(ClubType club, SkillLevel skill);
    
    // Get typical variation ranges for a skill level
    static VariationRange getVariationRange(SkillLevel skill);
    
    // Convert club type to/from string
    static std::string clubTypeToString(ClubType club);
    static ClubType stringToClubType(const std::string& clubStr);
    
    // Get club loft angle
    static double getClubLoft(ClubType club);
    
    // Get typical club length
    static double getClubLength(ClubType club);
    
private:
    // Initialize baseline data tables
    static void initializeData();
    
    // Baseline data tables
    static std::map<ClubType, std::map<SkillLevel, BaselineShotData>> baselineData;
    static std::map<SkillLevel, VariationRange> variationRanges;
    static std::map<ClubType, double> clubLofts;
    static std::map<ClubType, double> clubLengths;
    
    // Initialization flag
    static bool isInitialized;
};

} // namespace data
} // namespace gptgolf
