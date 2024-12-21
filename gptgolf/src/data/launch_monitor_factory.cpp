#include "data/launch_monitor.h"
#include "data/trackman_monitor.h"
#include "data/gcquad_monitor.h"
#include <map>
#include <stdexcept>

namespace gptgolf {
namespace data {

std::unique_ptr<ILaunchMonitor> LaunchMonitorFactory::create(const std::string& deviceType) {
    if (deviceType == "TrackMan") {
        return std::make_unique<TrackManMonitor>();
    } else if (deviceType == "GCQuad") {
        return std::make_unique<GCQuadMonitor>();
    }
    
    // Add support for other launch monitors here
    // else if (deviceType == "FlightScope") {
    //     return std::make_unique<FlightScopeMonitor>();
    // }
    
    throw std::runtime_error("Unsupported launch monitor type: " + deviceType);
}

std::vector<std::string> LaunchMonitorFactory::getSupportedDevices() {
    return {
        "TrackMan",
        "GCQuad"
        // Add other supported devices here
        // "FlightScope"
    };
}

// Helper function to get device capabilities
std::map<std::string, std::vector<std::string>> getDeviceCapabilities(const std::string& deviceType) {
    std::map<std::string, std::vector<std::string>> capabilities;
    
    if (deviceType == "TrackMan") {
        capabilities["features"] = {
            "Ball Data",
            "Club Data",
            "Weather Integration",
            "Indoor/Outdoor",
            "Normalization"
        };
        capabilities["ball_metrics"] = {
            "Ball Speed",
            "Launch Angle",
            "Launch Direction",
            "Spin Rate",
            "Spin Axis",
            "Carry Distance",
            "Total Distance"
        };
        capabilities["club_metrics"] = {
            "Club Speed",
            "Club Path",
            "Face Angle",
            "Attack Angle",
            "Dynamic Loft"
        };
    } else if (deviceType == "GCQuad") {
        capabilities["features"] = {
            "Ball Data",
            "Club Data",
            "Quadruplex Cameras",
            "High Speed Mode",
            "Ball Model Selection",
            "Indoor/Outdoor",
            "Normalization"
        };
        capabilities["ball_metrics"] = {
            "Ball Speed",
            "Launch Angle",
            "Launch Direction",
            "Total Spin",
            "Back Spin",
            "Side Spin",
            "Rifle Spin",
            "Carry Distance",
            "Total Distance",
            "Descent Angle"
        };
        capabilities["club_metrics"] = {
            "Club Speed",
            "Club Path",
            "Face Angle",
            "Attack Angle",
            "Dynamic Loft",
            "Face to Path",
            "Lie Angle",
            "Closure Rate"
        };
        capabilities["ball_models"] = {
            "ProV1",
            "ProV1x",
            "TP5",
            "TP5x",
            "ChromeSoft",
            "Generic"
        };
    }
    
    return capabilities;
}

// Helper function to get recommended settings
std::map<std::string, std::string> getRecommendedSettings(const std::string& deviceType, 
                                                         const std::string& environment) {
    std::map<std::string, std::string> settings;
    
    if (deviceType == "TrackMan") {
        settings["units"] = "Metric";
        settings["normalize"] = "true";
        if (environment == "Indoor") {
            settings["capture_rate"] = "40";  // Hz
        } else {
            settings["capture_rate"] = "20";  // Hz for outdoor
        }
    } else if (deviceType == "GCQuad") {
        settings["units"] = "Metric";
        settings["normalize"] = "true";
        settings["quadruplex"] = "true";
        settings["ballModel"] = "ProV1";
        if (environment == "Indoor") {
            settings["highSpeed"] = "true";
            settings["captureRate"] = "10000";  // Hz
        } else {
            settings["highSpeed"] = "false";
            settings["captureRate"] = "5000";   // Hz for outdoor
        }
    }
    
    return settings;
}

} // namespace data
} // namespace gptgolf
