#include "physics/physics.h"
#include "physics/trajectory.h"
#include "physics/physics_validation.h"
#include "physics/vector3d.h"
#include <cmath>

namespace gptgolf {
namespace physics {

TrajectoryResultWithStatus calculateTrajectoryWithValidation(
    double initialSpeed, double launchAngle, double spinRate,
    double windSpeed, double windAngle, const SpinAxis& spinAxis) {
    
    try {
        // Validate input parameters
        validation::validateLaunchParameters(initialSpeed, launchAngle, 
                                          spinRate, windSpeed, windAngle);
        
        // Initialize trajectory with preallocated memory
        TrajectoryResult result;
        result.trajectory.reserve(1000);
        // Constants for adaptive timestep - optimized for accuracy and performance
        const double MIN_TIMESTEP = 0.00005;  // 0.05ms for critical phases (launch/impact)
        const double MAX_TIMESTEP = 0.003;    // 3.0ms for stable mid-flight
        const double BASE_TIMESTEP = 0.0003;  // 0.3ms baseline for better precision
        const double VELOCITY_SCALE = 0.045;  // More aggressive velocity adaptation
        const double ACCEL_SCALE = 0.15;     // Reduced for smoother transitions
        const double SPIN_SCALE = 0.0002;    // Enhanced spin influence
        const double HEIGHT_SCALE = 0.2;     // Improved ground approach handling
        const double SMOOTH_FACTOR = 0.8;    // Better balance of responsiveness/smoothness
        const size_t MAX_ITERATIONS = 10000;  // Maximum number of iterations
        
        // Convert launch angle to radians
        double angleRad = launchAngle * M_PI / 180.0;
        
        // Initial velocity components
        double vx = initialSpeed * cos(angleRad);
        double vy = initialSpeed * sin(angleRad);
        
        // Initialize timestep tracking (non-static to avoid state persistence between calls)
        double prevTimeStep = BASE_TIMESTEP;
        Vector3D prevVelocity(vx, vy);
        
        // Current position
        double x = 0.0;
        double y = 0.0;
        double prevY = 0.0;  // For interpolating ground intersection
        
        result.trajectory.push_back(TrajectoryPoint(x, y));
        result.apex = 0.0;
    
        // Track velocity magnitude for adaptive timestep
        double velocityMagnitude = sqrt(vx * vx + vy * vy); // Used for adaptive timestep
        double timeStep = MIN_TIMESTEP;
        size_t iterationCount = 0;
        
        // Simulation loop with adaptive timestep
        while ((y >= 0.0 || result.trajectory.empty()) && iterationCount++ < MAX_ITERATIONS) {
            prevY = y;
            
            // Calculate relative velocity components with wind gradient
            double effectiveWindSpeed = getWindGradient(windSpeed, y);
            double relVx, relVy;
            calculateRelativeVelocity(vx, vy, effectiveWindSpeed, windAngle, relVx, relVy);
            double relV = sqrt(relVx * relVx + relVy * relVy);
            
            // Validate physical quantities
            validation::validatePhysicalQuantity(relV, 0.0, 200.0, "Relative velocity");
        
            // Calculate acceleration for timestep adaptation
            Vector3D currentVelocity(vx, vy);
            Vector3D acceleration = (currentVelocity - prevVelocity).scale(1.0 / prevTimeStep);
            double accelMagnitude = acceleration.magnitude();
            
            // Enhanced phase detection with improved transitions
            double launchProgress = std::min(1.0, result.trajectory.size() / 12.0); // Faster launch phase detection
            double landingFactor = (vy < 0) ? std::max(0.0, std::min(1.0, std::pow(y / 5.0, 0.8))) : 1.0; // More gradual ground approach
            
            // Optimized scaling factors with improved response curves
            double velocityFactor = std::exp(-VELOCITY_SCALE * std::pow(relV, 0.85)); // More sensitive to high velocities
            
            // Smoother acceleration response
            double accelFactor = std::exp(-ACCEL_SCALE * std::pow(accelMagnitude, 0.8));
            
            // Enhanced spin influence with better scaling
            double spinFactor = std::exp(-SPIN_SCALE * std::pow(spinRate, 0.9));
            
            // Improved ground approach with progressive scaling
            double heightFactor = std::min(1.0, std::pow(1.0 - std::exp(-HEIGHT_SCALE * y), 0.85));
            
            // Progressive phase-specific adjustments
            double phaseFactor = 0.4 + (0.6 * launchProgress * landingFactor);
            
            // Combine all factors for raw timestep
            double rawTimeStep = BASE_TIMESTEP * 
                               velocityFactor * 
                               accelFactor * 
                               spinFactor * 
                               heightFactor * 
                               phaseFactor;
            
            // Apply smoothing to prevent abrupt changes
            timeStep = (SMOOTH_FACTOR * prevTimeStep) + 
                      ((1.0 - SMOOTH_FACTOR) * rawTimeStep);
            
            // Clamp to limits
            timeStep = std::min(MAX_TIMESTEP, std::max(MIN_TIMESTEP, timeStep));
            
            // Update state for next iteration
            prevTimeStep = timeStep;
            prevVelocity = currentVelocity;
        
            if (relV > 0.001) {  // Avoid division by zero
                // Calculate Reynolds number and drag coefficient
                double reynoldsNumber = calculateReynoldsNumber(relV, y);
                double dragCoef = calculateDragCoefficient(reynoldsNumber);
                
                // Calculate air density at current altitude
                double density = getAirDensity(nullptr, y);
                
                // Validate physical quantities
                validation::validatePhysicalQuantity(reynoldsNumber, 0.0, 1e6, "Reynolds number");
                validation::validatePhysicalQuantity(dragCoef, 0.0, 1.0, "Drag coefficient");
                validation::validatePhysicalQuantity(density, 0.5, 1.5, "Air density");
                
                // Calculate drag force
                double dragForce = 0.5 * density * dragCoef * BALL_AREA * relV * relV;
                
                // Use simulation time for spin decay
                double elapsedTime = x / initialSpeed; // Approximate time based on distance/initial speed
                
                double magnusForce = calculateMagnusForce(spinRate, relV, BALL_RADIUS,
                                                        spinAxis, elapsedTime);
                
                validation::validatePhysicalQuantity(dragForce, 0.0, 100.0, "Drag force");
                validation::validatePhysicalQuantity(magnusForce, -50.0, 50.0, "Magnus force");
            
                // Apply forces
                double invRelV = 1.0 / relV;
                double dragAx = -dragForce * relVx * invRelV / BALL_MASS;
                double dragAy = -dragForce * relVy * invRelV / BALL_MASS;
                
                // Enhanced Magnus effect with proper force decomposition
                double magnusAx = -magnusForce * relVy * invRelV / BALL_MASS;
                double magnusAy = magnusForce * relVx * invRelV / BALL_MASS;
                
                validation::validatePhysicalQuantity(dragAx, -1000.0, 1000.0, "Drag acceleration X");
                validation::validatePhysicalQuantity(dragAy, -1000.0, 1000.0, "Drag acceleration Y");
                validation::validatePhysicalQuantity(magnusAx, -500.0, 500.0, "Magnus acceleration X");
                validation::validatePhysicalQuantity(magnusAy, -500.0, 500.0, "Magnus acceleration Y");
                
                // Update velocities
                vx += (dragAx + magnusAx) * timeStep;
                vy += (dragAy + magnusAy - GRAVITY) * timeStep;
            } else {
                // If relative velocity is very small, only apply gravity
                vy -= GRAVITY * timeStep;
            }
        
            // Update position
            x += vx * timeStep;
            y += vy * timeStep;
            
            // Validate new position
            validation::validateTrajectoryPoint(TrajectoryPoint(x, y));
            
            // Record trajectory point with decimation
            if (result.trajectory.size() < 2 || 
                abs(x - result.trajectory.back().x) > 0.1 || 
                abs(y - result.trajectory.back().y) > 0.1) {
                result.trajectory.push_back(TrajectoryPoint(x, y));
            }
            
            // Update apex
            if (y > result.apex) {
                result.apex = y;
            }
        }
    
        // Check for convergence failure
        if (iterationCount >= MAX_ITERATIONS) {
            return TrajectoryResultWithStatus(
                TrajectoryStatus::ConvergenceFailure,
                "Trajectory calculation failed to converge within maximum iterations"
            );
        }
        
        // Interpolate final point to exactly hit ground
        if (!result.trajectory.empty() && y < 0.0 && prevY > 0.0) {
            double t = -prevY / (y - prevY);
            x = result.trajectory.back().x - t * vx * timeStep;
            result.trajectory.back() = TrajectoryPoint(x, 0.0);
        }
        
        // Set final distance
        result.distance = result.trajectory.back().x;
        
        return TrajectoryResultWithStatus(TrajectoryStatus::Success, "", result);
        
    } catch (const PhysicsValidationError& e) {
        return TrajectoryResultWithStatus(
            TrajectoryStatus::InvalidInput,
            e.what()
        );
    } catch (const std::exception& e) {
        return TrajectoryResultWithStatus(
            TrajectoryStatus::CalculationError,
            std::string("Calculation error: ") + e.what()
        );
    }
}

// Legacy function implementation
TrajectoryResult calculateTrajectory(
    double initialSpeed, double launchAngle, double spinRate,
    double windSpeed, double windAngle, const SpinAxis& spinAxis) {
    
    auto result = calculateTrajectoryWithValidation(
        initialSpeed, launchAngle, spinRate, windSpeed, windAngle, spinAxis);
    
    if (result.isSuccess()) {
        return *result.result;
    } else {
        // Return empty trajectory for backward compatibility
        return TrajectoryResult();
    }
}

} // namespace physics
} // namespace gptgolf
