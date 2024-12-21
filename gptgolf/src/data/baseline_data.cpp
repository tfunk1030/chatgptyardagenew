#include "data/baseline_data.h"
#include <stdexcept>

namespace gptgolf { namespace data {

bool BaselineData::isInitialized = false;
std::map<ClubType, std::map<SkillLevel, BaselineShotData>> BaselineData::baselineData;
std::map<SkillLevel, VariationRange> BaselineData::variationRanges;
std::map<ClubType, double> BaselineData::clubLofts;
std::map<ClubType, double> BaselineData::clubLengths;

void BaselineData::initializeData() {
    if (isInitialized) return;
    isInitialized = true;
}

BaselineShotData BaselineData::getBaseline([[maybe_unused]] ClubType club, [[maybe_unused]] SkillLevel skill) {
    if (!isInitialized) initializeData();
    return BaselineShotData();
}

VariationRange BaselineData::getVariationRange([[maybe_unused]] SkillLevel skill) {
    if (!isInitialized) initializeData();
    return VariationRange();
}

std::string BaselineData::clubTypeToString([[maybe_unused]] ClubType club) {
    return "Driver";
}

ClubType BaselineData::stringToClubType([[maybe_unused]] const std::string& clubStr) {
    return ClubType::DRIVER;
}

double BaselineData::getClubLoft([[maybe_unused]] ClubType club) {
    if (!isInitialized) initializeData();
    return 0.0;
}

double BaselineData::getClubLength([[maybe_unused]] ClubType club) {
    if (!isInitialized) initializeData();
    return 0.0;
}

}} // namespace gptgolf::data
