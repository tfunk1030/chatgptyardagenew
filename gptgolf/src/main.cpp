#include <iostream>
#include <iomanip>
#include "physics/physics.h"
#include "physics/trajectory.h"
#include "weather/weather_data.h"

namespace gptgolf {

// Structure to hold simulation parameters
struct SimulationParams {
    double initialSpeed;    // m/s
    double launchAngle;     // degrees
    double spinRate;        // rpm
    double windSpeed;       // mph
    double windAngle;       // degrees
    physics::SpinAxis spinAxis;

    bool validate() const {
        return initialSpeed > 0 && 
               launchAngle >= -90 && launchAngle <= 90 &&
               spinRate >= 0 &&
               windSpeed >= 0 &&
               windAngle >= 0 && windAngle <= 360;
    }
};

class TrajectorySimulator {
public:
    physics::TrajectoryResultWithStatus runSimulation(const SimulationParams& params) {
        return physics::calculateTrajectoryWithValidation(
            params.initialSpeed,
            params.launchAngle,
            params.spinRate,
            params.windSpeed,
            params.windAngle,
            params.spinAxis
        );
    }

    static SimulationParams createDefaultParams() {
        return SimulationParams{
            70.0,    // initialSpeed (m/s)
            12.0,    // launchAngle (degrees)
            3000.0,  // spinRate (rpm)
            10.0,    // windSpeed (mph)
            45.0,    // windAngle (degrees)
            physics::SpinAxis{} // default spin axis
        };
    }
};

class ResultDisplayManager {
public:
    void displayHeader() const {
        std::cout << "Testing Golf Physics System\n" << std::endl;
        std::cout << "\n=== Testing Trajectory Calculations ===\n" << std::endl;
    }

    void displayInputParameters(const SimulationParams& params) const {
        std::cout << std::fixed << std::setprecision(2)
                  << "Initial Speed: " << params.initialSpeed << " m/s\n"
                  << "Launch Angle: " << params.launchAngle << " degrees\n"
                  << "Spin Rate: " << params.spinRate << " rpm\n"
                  << "Wind Speed: " << params.windSpeed << " mph\n"
                  << "Wind Angle: " << params.windAngle << " degrees\n"
                  << std::endl;
    }

    void displayTrajectoryResults(const physics::TrajectoryResult& result) const {
        std::cout << "Results:\n"
                  << "Total Distance: " << result.distance << " m\n"
                  << "Apex Height: " << result.apex << " m\n"
                  << "\nTrajectory Points Sample:\n";
        displayTrajectoryPoints(result.trajectory);
    }

private:
    void displayTrajectoryPoints(const std::vector<physics::TrajectoryPoint>& trajectory) const {
        const size_t numPoints = trajectory.size();
        const size_t pointsToPrint = std::min(size_t(5), numPoints);
        
        for (size_t i = 0; i < pointsToPrint; ++i) {
            const auto& point = trajectory[i];
            std::cout << "Point " << i + 1 << ": "
                      << "X=" << point.x << "m, "
                      << "Y=" << point.y << "m\n";
        }
        
        if (numPoints > pointsToPrint) {
            std::cout << "...\n";
            const auto& lastPoint = trajectory.back();
            std::cout << "Final Point: "
                      << "X=" << lastPoint.x << "m, "
                      << "Y=" << lastPoint.y << "m\n";
        }
    }
};

class SimulationManager {
public:
    SimulationManager() : display(), simulator() {}

    int run() {
        display.displayHeader();
        
        // Create and validate parameters
        auto params = TrajectorySimulator::createDefaultParams();
        if (!params.validate()) {
            std::cerr << "Invalid simulation parameters" << std::endl;
            return 1;
        }

        // Display input parameters
        display.displayInputParameters(params);
        
        // Run simulation
        auto trajectoryResult = simulator.runSimulation(params);
        
        // Handle calculation errors
        if (!trajectoryResult.isSuccess()) {
            std::cerr << "Trajectory calculation failed: " 
                      << trajectoryResult.errorMessage << std::endl;
            return 1;
        }

        // Display results
        display.displayTrajectoryResults(trajectoryResult.result.value());
        
        return 0;
    }

private:
    ResultDisplayManager display;
    TrajectorySimulator simulator;
};

} // namespace gptgolf

int main() {
    gptgolf::SimulationManager manager;
    return manager.run();
}
