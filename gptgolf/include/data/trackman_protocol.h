#pragma once

#include "launch_monitor_protocol.h"
#include <vector>
#include <cstdint>
#include <optional>

namespace gptgolf {
namespace data {

class TrackManProtocol : public LaunchMonitorProtocol {
public:
    virtual ~TrackManProtocol() = default;

    // Command types
    enum class CommandType : uint16_t {
        INITIALIZE = 0x0001,
        START_TRACKING = 0x0002,
        STOP_TRACKING = 0x0003,
        GET_STATUS = 0x0004,
        SET_MODE = 0x0005,
        CALIBRATE = 0x0006
    };

    // Packet types
    enum class PacketType : uint16_t {
        SHOT_DATA = 0x0101,
        CLUB_DATA = 0x0102,
        STATUS = 0x0103,
        ERROR = 0x0104,
        CALIBRATION = 0x0105
    };

    struct TrackManPacket {
        uint32_t timestamp;
        PacketType type;
        uint16_t length;
        std::vector<uint8_t> payload;
        uint32_t checksum;
    };

    // Protocol-specific methods that need to be implemented
    virtual bool sendCommand(CommandType cmd, const std::vector<uint8_t>& params = {}) = 0;
    virtual std::optional<TrackManPacket> receivePacket() = 0;
    virtual bool parseShot(const TrackManPacket& packet, LaunchMonitorData& data) = 0;
    virtual bool parseClubData(const TrackManPacket& packet, ClubPathAnalysis& data) = 0;
};

} // namespace data
} // namespace gptgolf
