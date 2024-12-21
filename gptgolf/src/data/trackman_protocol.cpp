#include "data/launch_monitor_protocol.h"
#include <nlohmann/json.hpp>
#include <cstring>

using json = nlohmann::json;

namespace gptgolf {
namespace data {

bool TrackManProtocol::sendCommand([[maybe_unused]] CommandType cmd, const std::vector<uint8_t>& params) {
    std::vector<uint8_t> packet;
    packet.reserve(params.size());
    packet.insert(packet.end(), params.begin(), params.end());
    return send(packet);
}

std::optional<TrackManProtocol::TrackManPacket> TrackManProtocol::receivePacket() {
    auto headerData = receive(8);
    if (headerData.size() != 8) return std::nullopt;
    
    TrackManPacket packet;
    size_t offset = 0;
    
    std::memcpy(&packet.timestamp, headerData.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    uint16_t typeValue;
    std::memcpy(&typeValue, headerData.data() + offset, sizeof(uint16_t));
    packet.type = static_cast<PacketType>(typeValue);
    offset += sizeof(uint16_t);
    
    std::memcpy(&packet.length, headerData.data() + offset, sizeof(uint16_t));
    
    if (packet.length > 0) {
        auto payloadData = receive(packet.length);
        if (payloadData.size() != packet.length) return std::nullopt;
        packet.payload = std::move(payloadData);
    }
    
    auto checksumData = receive(sizeof(uint32_t));
    if (checksumData.size() != sizeof(uint32_t)) return std::nullopt;
    std::memcpy(&packet.checksum, checksumData.data(), sizeof(uint32_t));
    
    return packet;
}

bool TrackManProtocol::parseShot(const TrackManPacket& packet, LaunchMonitorData& data) {
    if (packet.type != PacketType::SHOT_DATA) return false;
    
    try {
        std::string jsonStr(packet.payload.begin(), packet.payload.end());
        auto j = json::parse(jsonStr);
        
        data.ballSpeed = j["ball"]["speed"].get<double>();
        data.launchAngle = j["ball"]["launch_angle"].get<double>();
        data.spinRate = j["ball"]["total_spin"].get<double>();
        data.launchDirection = j["ball"]["launch_direction"].get<double>();
        data.carryDistance = j["ball"]["carry"].get<double>();
        data.totalDistance = j["ball"]["total"].get<double>();
        
        if (j.contains("advanced")) {
            auto& advanced = j["advanced"];
            data.smashFactor = advanced["smash_factor"].get<double>();
            data.spinAxis = advanced["spin_axis"].get<double>();
            data.maxHeight = advanced["apex"].get<double>();
            data.landingAngle = advanced["descent_angle"].get<double>();
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool TrackManProtocol::parseClubData(const TrackManPacket& packet, ClubPathAnalysis& data) {
    if (packet.type != PacketType::CLUB_DATA) return false;
    
    try {
        std::string jsonStr(packet.payload.begin(), packet.payload.end());
        auto j = json::parse(jsonStr);
        
        data.attackAngle = j["attack_angle"].get<double>();
        data.swingPlane = j["swing_plane"].get<double>();
        data.pathDirection = j["club_path"].get<double>();
        data.faceAngle = j["face_angle"].get<double>();
        data.loft = j["dynamic_loft"].get<double>();
        data.lie = j["dynamic_lie"].get<double>();
        data.closureRate = j["face_rotation"].get<double>();
        
        if (j.contains("club_path_data")) {
            for (const auto& point : j["club_path_data"]) {
                ClubPathAnalysis::SwingPath pathPoint;
                pathPoint.x = point["x"].get<double>();
                pathPoint.y = point["y"].get<double>();
                pathPoint.z = point["z"].get<double>();
                pathPoint.vx = point["vx"].get<double>();
                pathPoint.vy = point["vy"].get<double>();
                pathPoint.vz = point["vz"].get<double>();
                pathPoint.time = point["time"].get<double>();
                data.clubHeadPath.push_back(pathPoint);
            }
        }
        
        if (j.contains("impact")) {
            auto& impact = j["impact"];
            data.impact.locationX = impact["location_x"].get<double>();
            data.impact.locationY = impact["location_y"].get<double>();
            data.impact.efficiency = impact["efficiency"].get<double>();
            data.impact.contactTime = impact["contact_time"].get<double>();
            data.impact.deflection = impact["deflection"].get<double>();
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

}} // namespace gptgolf::data
