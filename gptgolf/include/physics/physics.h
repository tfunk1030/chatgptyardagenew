#pragma once

#include <vector>
#include <cmath>
#include "weather/weather_data.h"
#include "physics/atmosphere.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @file physics.h
 * @brief Core physics engine for golf ball trajectory simulation
 *
 * This module implements the physics calculations required for accurate golf ball
 * flight simulation, including aerodynamics, Magnus effect, and environmental factors.
 * It uses real-world physics principles and empirical data from TrackMan to provide
 * highly accurate trajectory predictions.
 *
 * @note All calculations use SI units (meters, seconds, kilograms)
 */

namespace gptgolf {
namespace physics {

/**
 * @defgroup PhysicalConstants Physical Constants
 * @{
 */
const double GRAVITY = 9.81;  //!< Gravitational acceleration (m/s^2)
const double STANDARD_AIR_DENSITY = 1.225;  //!< Air density at sea level, 20°C (kg/m^3)
const double AIR_VISCOSITY = 1.81e-5;  //!< Dynamic viscosity of air at 20°C (kg/(m·s))
const double LAPSE_RATE = 0.0065;  //!< Temperature decrease with altitude (K/m)
/** @} */

/**
 * @defgroup TrackmanBaseline TrackMan 2024 Baseline Data
 * @brief Reference data from TrackMan for validation and calibration
 * @{
 */
const double TRACKMAN_DRIVER_SPEED = 73.152;  //!< Reference driver speed (m/s, 167 mph)
const double TRACKMAN_DRIVER_LAUNCH = 10.5;   //!< Reference launch angle (degrees)
const double TRACKMAN_DRIVER_SPIN = 2700.0;   //!< Reference backspin rate (rpm)
const double TRACKMAN_DRIVER_HEIGHT = 45.72;  //!< Reference carry height (m, 150 ft)
/** @} */

/**
 * @defgroup BallConstants Golf Ball Physical Properties
 * @{
 */
const double BALL_MASS = 0.0459;  //!< Mass of a standard golf ball (kg)
const double BALL_RADIUS = 0.0213; //!< Radius of a standard golf ball (m)
const double BALL_AREA = M_PI * BALL_RADIUS * BALL_RADIUS; //!< Cross-sectional area (m²)
/** @} */

/**
 * @defgroup AerodynamicConstants Aerodynamic Properties
 * @brief Constants for aerodynamic calculations
 * @{
 */
const double BASE_DRAG_COEFFICIENT = 0.47;    //!< Baseline drag coefficient (dimensionless)
const double BASE_LIFT_COEFFICIENT = 0.25;    //!< Baseline lift coefficient (dimensionless)
const double CRITICAL_REYNOLDS = 4.0e4;       //!< Reynolds number for laminar-turbulent transition
const double TURBULENT_REYNOLDS = 4.0e5;      //!< Reynolds number for fully turbulent flow
/** @} */

/**
 * @defgroup MagnusConstants Magnus Effect Properties
 * @brief Constants for spin-related effects
 * @{
 */
const double SPIN_DECAY_RATE = 0.045;         //!< Rate of spin decay per second
const double SURFACE_ROUGHNESS = 0.0014;      //!< Golf ball dimple depth (m)
const double MAX_LIFT_COEFFICIENT = 0.35;     //!< Maximum achievable lift coefficient
const double SPIN_FACTOR_SCALING = 1.2e-4;    //!< Scaling factor for spin effects
/** @} */

/**
 * @brief Represents the orientation of the ball's spin axis
 *
 * The spin axis determines how the Magnus force affects the ball's trajectory.
 * A vertical spin axis (tilt = 0) produces pure backspin, while a tilted axis
 * creates sidespin effects.
 */
struct SpinAxis {
    double tilt;      //!< Spin axis tilt angle from vertical (degrees)
    double direction; //!< Spin axis direction from target line (degrees)
    
    /**
     * @brief Construct a new Spin Axis object
     * @param t Tilt angle in degrees (0 = vertical)
     * @param d Direction angle in degrees (0 = target line)
     */
    SpinAxis(double t = 0.0, double d = 0.0)
        : tilt(t), direction(d) {}
};

/**
 * @brief Represents a single point in the ball's trajectory
 */
struct TrajectoryPoint {
    double x; //!< Distance from origin along target line (m)
    double y; //!< Height above ground (m)
    
    /**
     * @brief Construct a new Trajectory Point
     * @param x Distance in meters
     * @param y Height in meters
     */
    TrajectoryPoint(double x = 0.0, double y = 0.0)
        : x(x), y(y) {}
};

/**
 * @brief Contains the complete results of a trajectory calculation
 */
struct TrajectoryResult {
    std::vector<TrajectoryPoint> trajectory; //!< Series of points defining the flight path
    double distance; //!< Total carry distance (m)
    double apex;     //!< Maximum height reached (m)
    
    TrajectoryResult()
        : distance(0.0), apex(0.0) {}
};

/**
 * @brief Calculates the complete trajectory of a golf shot
 * 
 * This is the main entry point for trajectory calculations. It takes into account
 * all major physics effects including drag, lift, and the Magnus effect, as well
 * as environmental conditions.
 * 
 * @param initialSpeed Ball speed at launch (m/s)
 * @param launchAngle Vertical launch angle (degrees)
 * @param spinRate Initial ball spin rate (rpm)
 * @param windSpeed Wind speed (m/s)
 * @param windAngle Wind direction relative to target line (degrees)
 * @param spinAxis Spin axis orientation (optional)
 * @return TrajectoryResult containing the complete flight path and key metrics
 * 
 * @note Wind angle of 0 degrees means wind from behind, 180 degrees means headwind
 * @see SpinAxis for details on spin orientation
 */
TrajectoryResult calculateTrajectory(
    double initialSpeed,
    double launchAngle,
    double spinRate,
    double windSpeed,
    double windAngle,
    const SpinAxis& spinAxis = SpinAxis());

/**
 * @brief Calculates air density at a given altitude
 * @param weatherData Current weather conditions
 * @param altitude Height above sea level (m)
 * @return Air density in kg/m³
 */
double getAirDensity(const weather::WeatherData* weatherData, double altitude = 0.0);

/**
 * @brief Adjusts ball speed for wind effects
 * @param speed Ball speed relative to ground (m/s)
 * @param weatherData Current weather conditions
 * @param altitude Height above sea level (m)
 * @return Wind-adjusted speed in m/s
 */
double getWindAdjustedSpeed(double speed, const weather::WeatherData* weatherData, double altitude = 0.0);

/**
 * @brief Calculates relative velocity components accounting for wind
 * @param velocityX Ball velocity X component (m/s)
 * @param velocityY Ball velocity Y component (m/s)
 * @param windSpeed Wind speed (m/s)
 * @param windAngle Wind direction (degrees)
 * @param[out] relativeVelX Resulting X component of relative velocity
 * @param[out] relativeVelY Resulting Y component of relative velocity
 */
void calculateRelativeVelocity(
    double velocityX,
    double velocityY,
    double windSpeed,
    double windAngle,
    double& relativeVelX,
    double& relativeVelY);

/**
 * @brief Calculates Reynolds number for current conditions
 * @param velocity Relative air velocity (m/s)
 * @param altitude Height above sea level (m)
 * @return Reynolds number (dimensionless)
 */
double calculateReynoldsNumber(double velocity, double altitude = 0.0);

/**
 * @brief Calculates drag coefficient based on Reynolds number
 * @param reynoldsNumber Current Reynolds number
 * @return Drag coefficient (dimensionless)
 */
double calculateDragCoefficient(double reynoldsNumber);

/**
 * @brief Calculates Magnus force magnitude
 * @param spinRate Ball spin rate (rpm)
 * @param velocity Relative air velocity (m/s)
 * @param radius Ball radius (m)
 * @param spinAxis Spin axis orientation
 * @param time Time since launch (s)
 * @return Magnus force magnitude (N)
 */
double calculateMagnusForce(
    double spinRate,
    double velocity,
    double radius,
    const SpinAxis& spinAxis,
    double time);

/**
 * @brief Calculates spin rate decay over time
 * @param initialSpin Initial spin rate (rpm)
 * @param time Time since launch (s)
 * @return Current spin rate (rpm)
 */
double calculateSpinDecay(double initialSpin, double time);

/**
 * @brief Calculates lift coefficient based on spin rate and velocity
 * @param spinRate Current spin rate (rpm)
 * @param velocity Relative air velocity (m/s)
 * @return Lift coefficient (dimensionless)
 */
double calculateLiftCoefficient(double spinRate, double velocity);

/**
 * @brief Calculates wind speed at a given altitude
 * @param baseWindSpeed Wind speed at ground level (m/s)
 * @param altitude Height above ground (m)
 * @return Wind speed at specified altitude (m/s)
 */
double getWindGradient(double baseWindSpeed, double altitude);

} // namespace physics
} // namespace gptgolf
