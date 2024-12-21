#include "physics/wind.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Wind::Wind(double speed, double direction, WindProfile profile, TerrainParameters terrain)
    : speed(speed)
    , direction(direction)
    , profile(profile)
    , terrain(terrain)
{}

double Wind::calculateLogProfile(double height) const {
    if (height < terrain.roughnessLength) {
        return 0.0;  // No wind below roughness length
    }
    
    // Logarithmic wind profile equation
    return speed * (log(height / terrain.roughnessLength) / 
                   log(terrain.referenceHeight / terrain.roughnessLength));
}

double Wind::calculatePowerLawProfile(double height) const {
    if (height < terrain.roughnessLength) {
        return 0.0;
    }
    
    // Power law wind profile equation
    return speed * pow(height / terrain.referenceHeight, terrain.powerLawExponent);
}

double Wind::calculateEkmanProfile(double height, double& outDirection) const {
    if (height < terrain.roughnessLength) {
        outDirection = direction;
        return 0.0;
    }
    
    // Ekman spiral parameters
    double eddyViscosity = 15.0;  // Typical value in m²/s
    double heightScale = sqrt(2.0 * eddyViscosity / CORIOLIS_PARAMETER);
    
    // Calculate normalized height
    double z = height / heightScale;
    
    // Calculate speed and direction change
    double speedFactor = exp(-z) * sqrt(1.0 + 2.0 * cos(z) + z * z);
    double directionChange = atan2(sin(z), cos(z) + z) * 180.0 / M_PI;
    
    outDirection = direction + directionChange;
    return speed * speedFactor;
}

double Wind::getSpeedAtHeight(double height) const {
    switch (profile) {
        case WindProfile::CONSTANT:
            return speed;
            
        case WindProfile::LOGARITHMIC:
            return calculateLogProfile(height);
            
        case WindProfile::POWER_LAW:
            return calculatePowerLawProfile(height);
            
        case WindProfile::EKMAN_SPIRAL: {
            double unusedDirection;
            return calculateEkmanProfile(height, unusedDirection);
        }
            
        default:
            return speed;
    }
}

double Wind::getDirectionAtHeight(double height) const {
    if (profile == WindProfile::EKMAN_SPIRAL) {
        double outDirection;
        calculateEkmanProfile(height, outDirection);
        return outDirection;
    }
    return direction;
}

Point3D Wind::applyWindEffect(const Point3D& position, double ballVelocity) const {
    Point3D result = position;
    
    // Get wind conditions at current height
    double currentSpeed = getSpeedAtHeight(position.z);
    double currentDirection = getDirectionAtHeight(position.z);
    
    // Convert wind direction to radians
    double dirRad = currentDirection * M_PI / 180.0;
    
    // Calculate relative wind effect based on ball velocity
    // Enhanced formula considering height-dependent effects
    double heightFactor = std::min(1.0, position.z / 100.0);  // Max effect at 100m
    double relativeEffect = (currentSpeed / (ballVelocity + currentSpeed + 1.0)) * heightFactor;
    
    // Calculate wind components with enhanced precision
    double windX = -currentSpeed * cos(dirRad);  // Negative because wind direction is FROM
    double windY = currentSpeed * sin(dirRad);
    
    // Apply wind effect with terrain-dependent scaling
    double terrainFactor = 1.0 - (log(terrain.roughnessLength + 1.0) / 10.0);  // Reduce effect for rough terrain
    const double baseScaleFactor = 0.1;
    double scaleFactor = baseScaleFactor * terrainFactor;
    
    // Apply wind effect to all components
    result.x = position.x + windX * relativeEffect * scaleFactor;
    result.y = position.y + windY * relativeEffect * scaleFactor;
    
    // Small vertical component for complex terrain
    if (terrain.roughnessLength > 0.1) {  // Only for rough terrain
        double verticalFactor = 0.05 * (terrain.roughnessLength / 1.0);
        result.z = position.z + currentSpeed * verticalFactor * relativeEffect;
    }
    
    return result;
}

double Wind::getBaseSpeed() const {
    return speed;
}

double Wind::getBaseDirection() const {
    return direction;
}

void Wind::setProfile(WindProfile newProfile) {
    profile = newProfile;
}

WindProfile Wind::getProfile() const {
    return profile;
}

void Wind::setTerrain(const TerrainParameters& newTerrain) {
    terrain = newTerrain;
}

const TerrainParameters& Wind::getTerrain() const {
    return terrain;
}
