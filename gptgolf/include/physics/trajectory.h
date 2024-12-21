#pragma once

#include "physics/physics.h"
#include "physics/physics_validation.h"
#include <optional>
#include <string>

/**
 * @file trajectory.h
 * @brief Enhanced trajectory calculation with validation and error handling
 *
 * This module extends the basic trajectory calculations from physics.h with
 * additional validation, error handling, and status reporting. It provides
 * a more robust interface for trajectory calculations that can handle edge
 * cases and report detailed error information.
 */

namespace gptgolf {
namespace physics {

/**
 * @brief Status codes for trajectory calculation results
 *
 * These status codes provide detailed information about the outcome
 * of a trajectory calculation attempt.
 */
enum class TrajectoryStatus {
    Success,           //!< Calculation completed successfully
    InvalidInput,      //!< One or more input parameters were invalid
    CalculationError,  //!< An error occurred during calculation
    ConvergenceFailure //!< Numerical method failed to converge
};

/**
 * @brief Extended trajectory result with status information
 *
 * This structure combines the basic trajectory result with additional
 * status information and error reporting capabilities. It provides
 * a more complete picture of the calculation outcome, especially
 * useful for handling edge cases and error conditions.
 */
struct TrajectoryResultWithStatus {
    TrajectoryStatus status;      //!< Status code indicating calculation outcome
    std::string errorMessage;     //!< Detailed error message if calculation failed
    std::optional<TrajectoryResult> result; //!< Trajectory result if successful
    
    /**
     * @brief Construct a new Trajectory Result With Status object
     * 
     * @param s Status code for the calculation
     * @param msg Error message (empty if successful)
     * @param res Optional trajectory result
     */
    TrajectoryResultWithStatus(TrajectoryStatus s = TrajectoryStatus::Success, 
                             const std::string& msg = "", 
                             std::optional<TrajectoryResult> res = std::nullopt)
        : status(s), errorMessage(msg), result(res) {}
    
    /**
     * @brief Check if the calculation was successful
     * @return true if calculation succeeded, false otherwise
     */
    bool isSuccess() const { return status == TrajectoryStatus::Success; }
};

/**
 * @brief Calculate trajectory with enhanced validation and error handling
 *
 * This function provides a more robust interface for trajectory calculations,
 * with comprehensive input validation and detailed error reporting. It's
 * recommended over the basic calculateTrajectory for production use.
 *
 * @param initialSpeed Initial ball speed (m/s)
 * @param launchAngle Vertical launch angle (degrees)
 * @param spinRate Initial ball spin rate (rpm)
 * @param windSpeed Wind speed (m/s)
 * @param windAngle Wind direction relative to target line (degrees)
 * @param spinAxis Spin axis orientation
 * @return TrajectoryResultWithStatus containing either the trajectory result or error information
 *
 * @note Input validation includes:
 * - Speed must be positive and below physically possible limits
 * - Launch angle must be between -90 and 90 degrees
 * - Spin rate must be non-negative and below equipment limits
 * - Wind speed must be non-negative and below extreme weather thresholds
 * - Wind angle must be between 0 and 360 degrees
 *
 * @see TrajectoryStatus for possible status codes
 * @see physics::calculateTrajectory for the underlying calculation
 *
 * Example usage:
 * @code
 * auto result = calculateTrajectoryWithValidation(70.0, 12.0, 2500.0, 5.0, 180.0);
 * if (result.isSuccess()) {
 *     auto trajectory = result.result.value();
 *     // Process successful result
 * } else {
 *     std::cerr << "Error: " << result.errorMessage << std::endl;
 * }
 * @endcode
 */
TrajectoryResultWithStatus calculateTrajectoryWithValidation(
    double initialSpeed,
    double launchAngle,
    double spinRate,
    double windSpeed,
    double windAngle,
    const SpinAxis& spinAxis);

/**
 * @brief Legacy trajectory calculation function
 *
 * This function provides backward compatibility with older code.
 * For new code, use calculateTrajectoryWithValidation instead.
 *
 * @deprecated Use calculateTrajectoryWithValidation for better error handling
 *
 * @param initialSpeed Initial ball speed (m/s)
 * @param launchAngle Vertical launch angle (degrees)
 * @param spinRate Initial ball spin rate (rpm)
 * @param windSpeed Wind speed (m/s)
 * @param windAngle Wind direction relative to target line (degrees)
 * @param spinAxis Spin axis orientation
 * @return TrajectoryResult containing the calculated trajectory
 *
 * @warning This function may throw exceptions for invalid inputs
 * @see calculateTrajectoryWithValidation for a more robust alternative
 */
TrajectoryResult calculateTrajectory(
    double initialSpeed,
    double launchAngle,
    double spinRate,
    double windSpeed,
    double windAngle,
    const SpinAxis& spinAxis);

} // namespace physics
} // namespace gptgolf
