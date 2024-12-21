#pragma once

#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <array>
#include <chrono>
#include "data/launch_monitor.h"

/**
 * @file launch_monitor_protocol.h
 * @brief Network protocol implementations for launch monitor devices
 *
 * This module provides protocol-level implementations for communicating with
 * various launch monitor devices (TrackMan, GCQuad, etc.). It handles the
 * low-level details of device communication, packet formatting, and data
 * parsing specific to each manufacturer's protocol.
 */

namespace gptgolf {
namespace data {

// Forward declarations
struct AdvancedBallData;
struct ClubPathAnalysis;
struct ImpactData;

/**
 * @brief Base class for launch monitor network protocols
 *
 * Defines the common interface for all launch monitor protocol implementations.
 * Handles basic network communication, packet validation, and data formatting.
 */
class LaunchMonitorProtocol {
public:
    virtual ~LaunchMonitorProtocol() = default;

    /** @name Connection Management
     * Methods for managing the network connection to the device
     * @{
     */
    /**
     * @brief Establish connection to launch monitor
     * @param host Device hostname or IP address
     * @param port Network port number
     * @return true if connection successful
     */
    virtual bool connect(const std::string& host, int port) = 0;

    /**
     * @brief Close connection to launch monitor
     * @return true if disconnection successful
     */
    virtual bool disconnect() = 0;

    /**
     * @brief Check connection status
     * @return true if currently connected
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Set network operation timeout
     * @param timeout Maximum time to wait for operations
     */
    virtual void setTimeout(std::chrono::milliseconds timeout) = 0;
    /** @} */

    /** @name Data Transmission
     * Methods for sending and receiving raw data
     * @{
     */
    /**
     * @brief Send raw data to device
     * @param data Bytes to send
     * @return true if transmission successful
     */
    virtual bool send(const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Receive raw data from device
     * @param expectedSize Number of bytes expected
     * @return Received data bytes
     */
    virtual std::vector<uint8_t> receive(size_t expectedSize) = 0;
    /** @} */

protected:
    /** @name Utility Methods
     * Helper functions for packet handling
     * @{
     */
    /**
     * @brief Calculate packet checksum
     * @param data Packet data
     * @return 32-bit CRC checksum
     */
    static uint32_t calculateChecksum(const std::vector<uint8_t>& data);

    /**
     * @brief Encode data into protocol packet
     * @param type Packet type identifier
     * @param payload Packet payload data
     * @return Encoded packet bytes
     */
    static std::vector<uint8_t> encodePacket(uint16_t type, const std::vector<uint8_t>& payload);

    /**
     * @brief Validate received packet
     * @param packet Received packet data
     * @return true if packet is valid
     */
    static bool validatePacket(const std::vector<uint8_t>& packet);
    /** @} */
};

/**
 * @brief TrackMan-specific protocol implementation
 *
 * Implements the TrackMan proprietary protocol for device communication.
 * Handles TrackMan-specific packet formats, commands, and data structures.
 */
class TrackManProtocol : public LaunchMonitorProtocol {
public:
    /** @name Command Types
     * Available commands for TrackMan devices
     * @{
     */
    enum class CommandType : uint16_t {
        INITIALIZE = 0x0001,     //!< Initialize device
        START_TRACKING = 0x0002, //!< Begin shot tracking
        STOP_TRACKING = 0x0003,  //!< Stop shot tracking
        GET_STATUS = 0x0004,     //!< Query device status
        SET_MODE = 0x0005,       //!< Set operating mode
        CALIBRATE = 0x0006       //!< Run calibration routine
    };
    /** @} */

    /** @name Packet Types
     * TrackMan data packet identifiers
     * @{
     */
    enum class PacketType : uint16_t {
        SHOT_DATA = 0x0101,    //!< Ball and club data for shot
        CLUB_DATA = 0x0102,    //!< Detailed club path data
        STATUS = 0x0103,       //!< Device status information
        ERROR = 0x0104,        //!< Error condition report
        CALIBRATION = 0x0105   //!< Calibration data
    };
    /** @} */

    /**
     * @brief TrackMan packet structure
     *
     * Format for all TrackMan protocol packets
     */
    struct TrackManPacket {
        uint32_t timestamp;      //!< Packet timestamp (ms since epoch)
        PacketType type;         //!< Packet type identifier
        uint16_t length;         //!< Payload length in bytes
        std::vector<uint8_t> payload; //!< Packet payload data
        uint32_t checksum;       //!< CRC32 checksum
    };

    /**
     * @brief Send command to TrackMan device
     * @param cmd Command to send
     * @param params Optional command parameters
     * @return true if command sent successfully
     */
    bool sendCommand(CommandType cmd, const std::vector<uint8_t>& params = {});

    /**
     * @brief Receive packet from TrackMan device
     * @return Optional containing received packet if available
     */
    std::optional<TrackManPacket> receivePacket();

    /**
     * @brief Parse shot data from packet
     * @param packet Received packet
     * @param[out] data Shot data structure to fill
     * @return true if parsing successful
     */
    bool parseShot(const TrackManPacket& packet, LaunchMonitorData& data);

    /**
     * @brief Parse club data from packet
     * @param packet Received packet
     * @param[out] data Club data structure to fill
     * @return true if parsing successful
     */
    bool parseClubData(const TrackManPacket& packet, ClubPathAnalysis& data);
};

/**
 * @brief GCQuad-specific protocol implementation
 *
 * Implements the Foresight Sports protocol for GCQuad devices.
 * Handles camera control, image acquisition, and data processing.
 */
class GCQuadProtocol : public LaunchMonitorProtocol {
public:
    /**
     * @brief Camera frame data structure
     *
     * Contains image data and calibration information for a single camera frame
     */
    struct CameraFrame {
        uint64_t timestamp;      //!< Frame timestamp (µs)
        uint32_t cameraId;       //!< Camera identifier (0-3)
        uint32_t frameNumber;    //!< Sequential frame number
        uint16_t width;          //!< Image width in pixels
        uint16_t height;         //!< Image height in pixels
        uint8_t bitsPerPixel;    //!< Pixel depth
        std::vector<uint8_t> imageData; //!< Raw image data
        std::array<double, 9> calibrationMatrix; //!< Camera calibration matrix
    };

    /** @name Command Types
     * Available commands for GCQuad devices
     * @{
     */
    enum class CommandType : uint16_t {
        START_CAMERAS = 0x0001,    //!< Start image acquisition
        STOP_CAMERAS = 0x0002,     //!< Stop image acquisition
        SET_EXPOSURE = 0x0003,     //!< Set camera exposure time
        SET_FRAMERATE = 0x0004,    //!< Set acquisition frame rate
        TRIGGER_CALIBRATION = 0x0005, //!< Start calibration sequence
        GET_CALIBRATION = 0x0006   //!< Retrieve calibration data
    };
    /** @} */

    /** @name Camera Control
     * Methods for controlling the GCQuad cameras
     * @{
     */
    /**
     * @brief Start camera streaming
     * @param frameRate Desired frame rate (fps)
     * @return true if streaming started successfully
     */
    virtual bool startStreaming(uint32_t frameRate) = 0;

    /**
     * @brief Stop camera streaming
     * @return true if streaming stopped successfully
     */
    virtual bool stopStreaming() = 0;

    /**
     * @brief Capture set of frames from all cameras
     * @return Vector of captured frames
     */
    virtual std::vector<CameraFrame> captureFrames() = 0;

    /**
     * @brief Set camera operation mode
     * @param quadruplex true for simultaneous 4-camera capture
     * @return true if mode set successfully
     */
    virtual bool setCameraMode(bool quadruplex) = 0;

    /**
     * @brief Set camera exposure time
     * @param microseconds Exposure duration in µs
     * @return true if exposure set successfully
     */
    virtual bool setExposure(uint32_t microseconds) = 0;

    /**
     * @brief Run camera calibration sequence
     * @return true if calibration successful
     */
    virtual bool calibrateCameras() = 0;
    /** @} */
};

/**
 * @brief Advanced ball flight data
 *
 * Detailed ball behavior measurements and calculations
 */
struct AdvancedBallData {
    double compressionAtImpact;    //!< Ball compression at impact (mm)
    double energyTransfer;         //!< Energy transfer efficiency (%)
    double spinDecayRate;          //!< Rate of spin decay (rpm/s)
    double peakHeight;             //!< Maximum height reached (m)
    double descendAngle;           //!< Angle of descent (degrees)
    double landingSpeed;           //!< Speed at landing (m/s)
    double rollDistance;           //!< Distance after first bounce (m)
    
    /**
     * @brief Data for each ball bounce
     */
    struct BounceData {
        double x, y;               //!< Position of bounce (m)
        double speed;              //!< Speed at bounce (m/s)
        double angle;              //!< Angle at bounce (degrees)
        double height;             //!< Height of bounce (m)
    };
    std::vector<BounceData> bounces; //!< Sequence of bounces
    
    /**
     * @brief Detailed spin characteristics
     */
    struct SpinData {
        double backspinStart;      //!< Initial backspin (rpm)
        double backspinLanding;    //!< Backspin at landing (rpm)
        double sidespin;           //!< Sidespin (rpm)
        double tiltAxis;           //!< Spin axis tilt (degrees)
        double gyroscopicStability;//!< Gyroscopic stability factor
    } spin;
    
    /**
     * @brief Weather effects on ball flight
     */
    struct WeatherImpact {
        double windEffect;         //!< Wind effect on total distance (m)
        double densityEffect;      //!< Air density effect (%)
        double humidityEffect;     //!< Humidity effect (%)
        double altitudeEffect;     //!< Altitude effect (%)
    } weather;
};

/**
 * @brief Detailed club path analysis
 *
 * Comprehensive measurements of club movement through impact
 */
struct ClubPathAnalysis {
    double attackAngle;           //!< Angle of attack (degrees)
    double swingPlane;            //!< Swing plane angle (degrees)
    double lowPoint;              //!< Low point distance from ball (mm)
    double pathDirection;         //!< Path direction at impact (degrees)
    double faceAngle;            //!< Face angle at impact (degrees)
    double loft;                 //!< Dynamic loft at impact (degrees)
    double lie;                  //!< Dynamic lie at impact (degrees)
    double closureRate;          //!< Face closure rate (deg/s)
    
    /**
     * @brief Club head position and velocity data
     */
    struct SwingPath {
        double x, y, z;          //!< Position (m)
        double vx, vy, vz;       //!< Velocity (m/s)
        double time;             //!< Time relative to impact (s)
    };
    std::vector<SwingPath> clubHeadPath; //!< Club head trajectory
    
    /**
     * @brief Impact characteristics
     */
    struct ImpactData {
        double locationX;        //!< Horizontal impact location on face (mm)
        double locationY;        //!< Vertical impact location on face (mm)
        double efficiency;       //!< Impact efficiency (%)
        double contactTime;      //!< Ball-club contact time (ms)
        double deflection;       //!< Club face deflection (mm)
    } impact;
    
    /**
     * @brief Swing metrics
     */
    struct Metrics {
        double tempo;           //!< Backswing:downswing ratio
        double transition;      //!< Transition time (ms)
        double maxSpeed;        //!< Maximum club head speed (m/s)
        double speedAtImpact;   //!< Speed at impact (m/s)
        double acceleration;    //!< Acceleration through impact (m/s²)
    } metrics;
};

} // namespace data
} // namespace gptgolf
