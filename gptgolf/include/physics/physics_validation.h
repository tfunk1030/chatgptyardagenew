#pragma once

#include "physics/physics.h"
#include <stdexcept>
#include <string>
#include <cmath>

namespace gptgolf {
namespace physics {

// Custom exception classes for physics engine errors
class PhysicsValidationError : public std::runtime_error {
public:
    explicit PhysicsValidationError(const std::string& message) 
        : std::runtime_error(message) {}
};

// Input validation functions
namespace validation {

// Validate launch parameters
void validateLaunchParameters(
    double initialSpeed,
    double launchAngle, 
    double spinRate,
    double windSpeed,
    double windAngle) 
{
    if (initialSpeed < 0.0) {
        throw PhysicsValidationError("Initial speed cannot be negative");
    }
    if (initialSpeed > 100.0) { // ~225 mph, beyond physical limits
        throw PhysicsValidationError("Initial speed exceeds maximum physical limit");
    }
    if (launchAngle < -90.0 || launchAngle > 90.0) {
        throw PhysicsValidationError("Launch angle must be between -90 and 90 degrees");
    }
    if (spinRate < 0.0) {
        throw PhysicsValidationError("Spin rate cannot be negative");
    }
    if (spinRate > 10000.0) { // Maximum realistic spin rate
        throw PhysicsValidationError("Spin rate exceeds maximum physical limit");
    }
    if (windSpeed < 0.0) {
        throw PhysicsValidationError("Wind speed cannot be negative");
    }
    if (windSpeed > 50.0) { // ~112 mph, beyond typical golf conditions
        throw PhysicsValidationError("Wind speed exceeds maximum expected value");
    }
    if (windAngle < 0.0 || windAngle > 360.0) {
        throw PhysicsValidationError("Wind angle must be between 0 and 360 degrees");
    }
}

// Validate trajectory point
void validateTrajectoryPoint(const TrajectoryPoint& point, double maxDistance = 1000.0) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
        throw PhysicsValidationError("Non-finite values in trajectory calculation");
    }
    if (point.x < 0.0 || point.x > maxDistance) {
        throw PhysicsValidationError("Trajectory point X coordinate out of bounds");
    }
    if (point.y < -0.1 || point.y > 500.0) { // Allow slight negative for ground intersection
        throw PhysicsValidationError("Trajectory point Y coordinate out of bounds");
    }
}

// Validate physical quantities
void validatePhysicalQuantity(
    double value,
    double minValue,
    double maxValue, 
    const std::string& name) 
{
    if (!std::isfinite(value)) {
        throw PhysicsValidationError(name + " calculation resulted in non-finite value");
    }
    if (value < minValue || value > maxValue) {
        throw PhysicsValidationError(name + " out of valid range");
    }
}

} // namespace validation
} // namespace physics
} // namespace gptgolf
