#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include "data/launch_monitor.h"

namespace gptgolf {
namespace data {

/**
 * @brief Implementation of TrackMan launch monitor integration
 */
class TrackManMonitor : public LaunchMonitorBase {
public:
    TrackManMonitor();
    ~TrackManMonitor() override;

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
    static constexpr const char* DEFAULT_IP = "192.168.0.200";
    static constexpr int DEFAULT_PORT = 8888;
    std::string deviceIp_;
    int devicePort_;
    std::atomic<bool> connected_;
    std::atomic<bool> tracking_;
    
    // Data handling
    std::queue<LaunchMonitorData> dataQueue_;
    mutable std::mutex dataMutex_;
    std::thread dataThread_;
    std::atomic<bool> shouldStop_;

    // TrackMan-specific settings
    struct Settings {
        std::string units;          // "Metric" or "Imperial"
        bool clubData;              // Include club data in measurements
        bool ballData;              // Include ball data in measurements
        int captureRate;            // Data capture rate in Hz
        std::string environment;    // "Indoor" or "Outdoor"
        bool normalizeData;         // Apply TrackMan's normalization
    } settings_;

    // Private methods
    void dataCollectionThread();
    bool parseTrackManData(const std::string& rawData, LaunchMonitorData& data);
    bool validateTrackManData(const LaunchMonitorData& data) const;
    void applyNormalization(LaunchMonitorData& data) const;
    
    // TrackMan-specific calculations
    double calculateSmashFactor(double ballSpeed, double clubSpeed) const;
    double calculateSpinAxis(double sidespin, double backspin) const;
    void adjustForEnvironment(LaunchMonitorData& data) const;
};

} // namespace data
} // namespace gptgolf
