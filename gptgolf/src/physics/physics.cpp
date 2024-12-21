#include "physics/physics.h"
#include "weather/weather_data.h"
#include <cmath>

namespace gptgolf {
namespace physics {

double getAirDensity(const weather::WeatherData* weatherData, double altitude) {
    // Use the new atmospheric model for density calculations
    return gptgolf::physics::standardAtmosphere.getDensity(altitude, weatherData);
}

double calculateReynoldsNumber(double velocity, double altitude) {
    // Re = (ρvD)/μ where:
    // ρ = air density
    // v = velocity
    // D = characteristic length (ball diameter)
    // μ = dynamic viscosity of air
    
    double density = getAirDensity(nullptr, altitude);  // Use standard atmosphere model
    return (density * velocity * (2 * BALL_RADIUS)) / AIR_VISCOSITY;
}

double calculateDragCoefficient(double reynoldsNumber) {
    // Implement drag crisis transition
    if (reynoldsNumber < CRITICAL_REYNOLDS) {
        // Laminar flow regime
        return BASE_DRAG_COEFFICIENT;
    } else if (reynoldsNumber > TURBULENT_REYNOLDS) {
        // Fully turbulent regime
        return BASE_DRAG_COEFFICIENT * 0.5;
    } else {
        // Transition regime - smooth interpolation
        double t = (reynoldsNumber - CRITICAL_REYNOLDS) / 
                  (TURBULENT_REYNOLDS - CRITICAL_REYNOLDS);
        return BASE_DRAG_COEFFICIENT * (1.0 - 0.5 * t);
    }
}

double calculateSpinDecay(double initialSpin, double time) {
    // Exponential decay of spin rate over time
    // Uses the empirically determined SPIN_DECAY_RATE
    return initialSpin * exp(-SPIN_DECAY_RATE * time);
}

double calculateLiftCoefficient(double spinRate, double velocity) {
    // Calculate lift coefficient based on spin rate and velocity
    // Uses the golf ball's surface roughness and maximum lift coefficient
    
    // Calculate spin factor (non-dimensional)
    double spinFactor = (spinRate * M_PI / 30.0) * BALL_RADIUS / velocity;
    
    // Account for surface roughness effects
    double roughnessEffect = 1.0 + (SURFACE_ROUGHNESS / BALL_RADIUS);
    
    // Calculate lift coefficient with saturation at MAX_LIFT_COEFFICIENT
    double liftCoef = BASE_LIFT_COEFFICIENT * spinFactor * roughnessEffect;
    return std::min(liftCoef, MAX_LIFT_COEFFICIENT);
}

double calculateMagnusForce(double spinRate, double velocity, [[maybe_unused]] double radius,
                          const SpinAxis& spinAxis, double time) {
    // Enhanced Magnus force calculation including spin decay and axis orientation
    
    // Calculate decayed spin rate
    double currentSpin = calculateSpinDecay(spinRate, time);
    
    // Convert spin axis angles to radians
    double tiltRad = spinAxis.tilt * M_PI / 180.0;
    double directionRad = spinAxis.direction * M_PI / 180.0;
    
    // Calculate effective spin components
    double verticalSpin = currentSpin * cos(tiltRad);
    double horizontalSpin = currentSpin * sin(tiltRad);
    
    // Calculate lift coefficient based on current conditions
    double liftCoef = calculateLiftCoefficient(currentSpin, velocity);
    
    // Calculate Magnus force magnitude
    double forceMagnitude = 0.5 * STANDARD_AIR_DENSITY * BALL_AREA * 
                           liftCoef * velocity * velocity;
    
    // Adjust force based on spin axis orientation
    double verticalForce = forceMagnitude * (verticalSpin / currentSpin);
    double horizontalForce = forceMagnitude * (horizontalSpin / currentSpin);
    
    // Return total Magnus force (vertical component for typical backspin)
    return verticalForce * cos(directionRad) + horizontalForce * sin(directionRad);
}

double getWindGradient(double baseWindSpeed, double altitude) {
    // Wind speed increases with height following a power law profile
    // v(h) = v_ref * (h/h_ref)^α
    // where α is the Hellmann exponent (typically 0.143 for open terrain)
    
    const double referenceHeight = 10.0;  // Standard reference height in meters
    const double hellmannExponent = 0.143;  // For open terrain
    
    if (altitude < 0.1) return baseWindSpeed;  // Avoid division by zero or negative values
    
    return baseWindSpeed * pow(altitude / referenceHeight, hellmannExponent);
}

void calculateRelativeVelocity(double velocityX, double velocityY,
                             double windSpeed, double windAngle,
                             double& relativeVelX, double& relativeVelY) {
    // Convert wind angle to radians
    double windAngleRad = windAngle * M_PI / 180.0;
    
    // Calculate wind components
    double windVx = windSpeed * cos(windAngleRad);
    double windVy = windSpeed * sin(windAngleRad);
    
    // Calculate relative velocity components
    relativeVelX = velocityX - windVx;
    relativeVelY = velocityY - windVy;
}

double getWindAdjustedSpeed(double speed, const weather::WeatherData* weatherData, double altitude) {
    if (!weatherData) {
        return speed;
    }
    
    // Get the air density ratio compared to standard conditions
    double actualDensity = getAirDensity(weatherData, altitude);
    double standardDensity = gptgolf::physics::standardAtmosphere.getDensity(altitude, nullptr);
    double densityRatio = actualDensity / standardDensity;
    
    // Adjust wind speed based on air density
    return speed * sqrt(densityRatio);
}

} // namespace physics
} // namespace gptgolf
