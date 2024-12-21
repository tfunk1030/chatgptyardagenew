#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include "data/launch_monitor.h"

namespace gptgolf {
namespace data {

/**
 * @brief Implementation of Foresight GCQuad launch monitor integration
 */
class GCQuadMonitor : public LaunchMonitorBase {
public:
    GCQuadMonitor();
    ~GCQuadMonitor() override;

    // ILaunchMonitor interface implementation
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    std::string getDeviceInfo() const override;

    std::optional<LaunchMonitorData> getLastShot() override;
    bool startTracking() override;
    bool stopTracking() override;
    bool isTracking() const override;

    bool configure(const std::string& setting, const std::string& value) override;
    std::string getSetting(const std::string& setting) const override;

private:
    // Device connection details
    static constexpr const char* DEFAULT_IP = "192.168.0.100";
    static constexpr int DEFAULT_PORT = 2001;  // GCQuad default port
    std::string deviceIp_;
    int devicePort_;
    std::atomic<bool> connected_;
    std::atomic<bool> tracking_;
    
    // Data handling
    std::queue<LaunchMonitorData> dataQueue_;
    mutable std::mutex dataMutex_;
    std::thread dataThread_;
    std::atomic<bool> shouldStop_;

    // GCQuad-specific settings
    struct Settings {
        std::string units;          // "Metric" or "Imperial"
        bool clubData;              // Include club data in measurements
        bool ballData;              // Include ball data in measurements
        std::string environment;    // "Indoor" or "Outdoor"
        bool quadruplex;            // Use all 4 cameras (true) or just 2 (false)
        bool normalizeData;         // Apply normalization
        std::string ballModel;      // Specific ball model for spin calculations
        int captureRate;            // Data capture rate in Hz
        bool highSpeed;             // High-speed camera mode
    } settings_;

    // Private methods
    void dataCollectionThread();
    bool parseGCQuadData(const std::string& rawData, LaunchMonitorData& data);
    bool validateGCQuadData(const LaunchMonitorData& data) const;
    void applyNormalization(LaunchMonitorData& data) const;
    
    // GCQuad-specific calculations
    double calculateTotalSpin(double backspin, double sidespin, double riflespin) const;
    double calculateSpinAxis(double backspin, double sidespin) const;
    void adjustForBallModel(LaunchMonitorData& data) const;
    double calculateBallCompression(double ballSpeed, const std::string& ballModel) const;
    
    // Camera calibration
    bool calibrateCameras();
    bool checkCalibration() const;
    void updateCalibrationStatus();
    
    // Quality metrics
    double calculateConfidence(const LaunchMonitorData& data) const;
    std::string determineQualityRating(const LaunchMonitorData& data) const;
    bool validateImageQuality(double brightness, double contrast) const;
};

} // namespace data
} // namespace gptgolf
