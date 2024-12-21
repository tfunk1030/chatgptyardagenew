#pragma once

#include "launch_monitor_protocol.h"
#include <vector>
#include <cstdint>
#include <array>
#include <chrono>

namespace gptgolf {
namespace data {

class GCQuadProtocol : public LaunchMonitorProtocol {
public:
    virtual ~GCQuadProtocol() = default;

    // Camera frame structure
    struct CameraFrame {
        uint64_t timestamp;
        uint32_t cameraId;
        uint32_t frameNumber;
        uint16_t width;
        uint16_t height;
        uint8_t bitsPerPixel;
        std::vector<uint8_t> imageData;
        std::array<double, 9> calibrationMatrix;
    };

    // Command types
    enum class CommandType : uint16_t {
        START_CAMERAS = 0x0001,
        STOP_CAMERAS = 0x0002,
        SET_EXPOSURE = 0x0003,
        SET_FRAMERATE = 0x0004,
        TRIGGER_CALIBRATION = 0x0005,
        GET_CALIBRATION = 0x0006
    };

    // Protocol-specific methods that need to be implemented
    virtual bool startStreaming(uint32_t frameRate) = 0;
    virtual bool stopStreaming() = 0;
    virtual std::vector<CameraFrame> captureFrames() = 0;
    virtual bool setCameraMode(bool quadruplex) = 0;
    virtual bool setExposure(uint32_t microseconds) = 0;
    virtual bool calibrateCameras() = 0;
};

} // namespace data
} // namespace gptgolf
