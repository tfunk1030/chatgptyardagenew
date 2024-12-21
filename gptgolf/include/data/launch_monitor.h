#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <ctime>
#include "storage.h"

/**
 * @file launch_monitor.h
 * @brief Launch monitor integration and data processing interface
 *
 * This module provides interfaces and base implementations for integrating
 * various launch monitor devices (e.g., TrackMan, GCQuad) into the system.
 * It handles device communication, data acquisition, and conversion of
 * launch monitor specific data formats into our standardized internal format.
 */

namespace gptgolf {
namespace data {

/**
 * @brief Comprehensive launch monitor measurement data
 *
 * Contains all possible measurements that can be captured by modern launch
 * monitors. Not all devices will provide all measurements - unavailable
 * measurements will be set to 0 or empty strings.
 */
struct LaunchMonitorData {
    /** @name Ball Flight Characteristics
     * Core measurements related to ball flight
     * @{
     */
    double ballSpeed;        //!< Ball speed immediately after impact (m/s)
    double launchAngle;      //!< Vertical launch angle (degrees, positive = upward)
    double launchDirection;  //!< Horizontal launch direction (degrees, 0 = target line, positive = right)
    double spinRate;         //!< Total ball spin rate (rpm)
    double spinAxis;         //!< Spin axis tilt (degrees, -90 to 90, 0 = pure backspin, positive = right spin)
    /** @} */

    /** @name Advanced Ball Data
     * Derived measurements and landing characteristics
     * @{
     */
    double smashFactor;      //!< Energy transfer efficiency (ball speed / club speed ratio)
    double ballVertical;     //!< Vertical deviation from target at landing (m, positive = long)
    double ballHorizontal;   //!< Horizontal deviation from target at landing (m, positive = right)
    double carryDistance;    //!< Distance traveled through the air (m)
    double totalDistance;    //!< Total distance including roll (m)
    double maxHeight;        //!< Maximum height reached during flight (m)
    double landingAngle;     //!< Descent angle at landing (degrees)
    /** @} */

    /** @name Club Delivery Data
     * Measurements of club head characteristics at impact
     * @{
     */
    double clubSpeed;        //!< Club head speed at impact (m/s)
    double clubPath;         //!< Club path direction relative to target line (degrees)
    double faceAngle;        //!< Club face angle relative to target line (degrees)
    double attackAngle;      //!< Club head vertical approach angle (degrees)
    double dynamicLoft;      //!< Effective loft at impact (degrees)
    /** @} */

    /** @name Quality Metrics
     * Data quality and confidence indicators
     * @{
     */
    double confidence;       //!< Measurement confidence level (0-1)
    std::string quality;     //!< Overall quality rating ("Good", "Partial", "Poor")
    /** @} */

    /**
     * @brief Default constructor initializing all numeric values to 0
     */
    LaunchMonitorData()
        : ballSpeed(0), launchAngle(0), launchDirection(0), spinRate(0), spinAxis(0),
          smashFactor(0), ballVertical(0), ballHorizontal(0), carryDistance(0),
          totalDistance(0), maxHeight(0), landingAngle(0), clubSpeed(0), clubPath(0),
          faceAngle(0), attackAngle(0), dynamicLoft(0), confidence(0) {}
};

/**
 * @brief Interface for launch monitor device integration
 *
 * This interface defines the contract that all launch monitor implementations
 * must fulfill. It provides methods for device connection, data acquisition,
 * and configuration.
 */
class ILaunchMonitor {
public:
    virtual ~ILaunchMonitor() = default;

    /** @name Device Management
     * Methods for controlling the launch monitor device
     * @{
     */
    /**
     * @brief Establish connection with the launch monitor
     * @return true if connection successful, false otherwise
     */
    virtual bool connect() = 0;

    /**
     * @brief Terminate connection with the launch monitor
     * @return true if disconnection successful, false otherwise
     */
    virtual bool disconnect() = 0;

    /**
     * @brief Check connection status
     * @return true if device is connected, false otherwise
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Get device information and status
     * @return String containing device details (model, firmware version, etc.)
     */
    virtual std::string getDeviceInfo() const = 0;
    /** @} */

    /** @name Data Acquisition
     * Methods for retrieving shot data from the device
     * @{
     */
    /**
     * @brief Retrieve data from the most recent shot
     * @return Optional containing shot data if available
     */
    virtual std::optional<LaunchMonitorData> getLastShot() = 0;

    /**
     * @brief Begin monitoring for shots
     * @return true if tracking started successfully
     */
    virtual bool startTracking() = 0;

    /**
     * @brief Stop monitoring for shots
     * @return true if tracking stopped successfully
     */
    virtual bool stopTracking() = 0;

    /**
     * @brief Check if device is currently monitoring
     * @return true if device is actively tracking shots
     */
    virtual bool isTracking() const = 0;
    /** @} */

    /** @name Device Configuration
     * Methods for configuring device settings
     * @{
     */
    /**
     * @brief Configure a device setting
     * @param setting Setting identifier
     * @param value New setting value
     * @return true if configuration successful
     */
    virtual bool configure(const std::string& setting, const std::string& value) = 0;

    /**
     * @brief Retrieve current value of a setting
     * @param setting Setting identifier
     * @return Current value of the setting
     */
    virtual std::string getSetting(const std::string& setting) const = 0;
    /** @} */

    /**
     * @brief Convert device-specific data to internal format
     * @param data Raw launch monitor data
     * @return Converted shot data in internal format
     */
    virtual ShotData convertToShotData(const LaunchMonitorData& data) = 0;
};

/**
 * @brief Base implementation of common launch monitor functionality
 *
 * Provides default implementations for data conversion and validation
 * that can be inherited by specific launch monitor implementations.
 */
class LaunchMonitorBase : public ILaunchMonitor {
public:
    /**
     * @brief Convert launch monitor data to internal shot data format
     * @param data Raw launch monitor data
     * @return Converted shot data
     */
    ShotData convertToShotData(const LaunchMonitorData& data) override {
        ShotData shot;
        shot.initialVelocity = data.ballSpeed;
        shot.spinRate = data.spinRate;
        shot.launchAngle = data.launchAngle;
        shot.actualDistance = data.carryDistance;
        shot.lateralDeviation = data.ballHorizontal;
        shot.timestamp = std::time(nullptr);
        return shot;
    }

protected:
    /**
     * @brief Validate ball flight measurements
     * @param data Launch monitor data to validate
     * @return true if ball data is within valid ranges
     */
    bool validateBallData(const LaunchMonitorData& data) const {
        return data.ballSpeed > 0 && 
               data.ballSpeed < 100.0 &&  // Max ~220mph
               data.launchAngle >= -10.0 && 
               data.launchAngle <= 60.0 &&
               data.spinRate >= 0 &&
               data.spinRate <= 12000.0;  // Max reasonable spin rate
    }

    /**
     * @brief Validate club delivery measurements
     * @param data Launch monitor data to validate
     * @return true if club data is within valid ranges
     */
    bool validateClubData(const LaunchMonitorData& data) const {
        return data.clubSpeed > 0 &&
               data.clubSpeed < 67.0 &&  // Max ~150mph
               data.smashFactor >= 1.0 &&
               data.smashFactor <= 1.5;  // Reasonable smash factor range
    }
};

/**
 * @brief Factory for creating launch monitor instances
 *
 * Provides centralized creation of launch monitor objects based on
 * device type, handling initialization and configuration.
 */
class LaunchMonitorFactory {
public:
    /**
     * @brief Create a new launch monitor instance
     * @param deviceType Type of launch monitor to create (e.g., "TrackMan", "GCQuad")
     * @return Unique pointer to created launch monitor instance
     * @throws std::runtime_error if device type is not supported
     */
    static std::unique_ptr<ILaunchMonitor> create(const std::string& deviceType);

    /**
     * @brief Get list of supported launch monitor types
     * @return Vector of supported device type strings
     */
    static std::vector<std::string> getSupportedDevices();
};

} // namespace data
} // namespace gptgolf
