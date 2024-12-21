#include <gtest/gtest.h>
#include "data/launch_monitor_protocol.h"
#include <thread>
#include <chrono>
#include <boost/asio.hpp>

using namespace gptgolf::data;
using boost::asio::ip::tcp;

// Mock server for testing protocols
class MockLaunchMonitorServer {
public:
    MockLaunchMonitorServer(int port) : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }
    
    void run() {
        serverThread_ = std::thread([this]() {
            io_context_.run();
        });
    }
    
    void stop() {
        io_context_.stop();
        if (serverThread_.joinable()) {
            serverThread_.join();
        }
    }
    
    void sendShotData(const std::string& data) {
        if (socket_) {
            boost::asio::write(*socket_, boost::asio::buffer(data));
        }
    }
    
private:
    void start_accept() {
        socket_ = std::make_shared<tcp::socket>(io_context_);
        acceptor_.async_accept(*socket_,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    // Handle connection
                    start_read();
                }
                start_accept();
            });
    }
    
    void start_read() {
        auto buffer = std::make_shared<std::vector<uint8_t>>(1024);
        socket_->async_read_some(
            boost::asio::buffer(*buffer),
            [this, buffer](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    handleCommand(*buffer, length);
                    start_read();
                }
            });
    }
    
    void handleCommand(const std::vector<uint8_t>& data, size_t length) {
        // Mock response based on command
        if (length >= 8) {
            uint16_t command;
            std::memcpy(&command, data.data() + 4, sizeof(uint16_t));
            
            switch (command) {
                case 0x0001: // INITIALIZE
                    sendResponse("{\"status\":\"ok\",\"protocol_version\":1}");
                    break;
                case 0x0002: // START_TRACKING
                    sendResponse("{\"status\":\"ok\",\"tracking\":true}");
                    break;
                case 0x0003: // STOP_TRACKING
                    sendResponse("{\"status\":\"ok\",\"tracking\":false}");
                    break;
            }
        }
    }
    
    void sendResponse(const std::string& response) {
        if (socket_) {
            boost::asio::write(*socket_, boost::asio::buffer(response));
        }
    }
    
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::shared_ptr<tcp::socket> socket_;
    std::thread serverThread_;
};

class ProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_ = std::make_unique<MockLaunchMonitorServer>(12345);
        server_->run();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for server startup
    }
    
    void TearDown() override {
        server_->stop();
    }
    
    std::unique_ptr<MockLaunchMonitorServer> server_;
};

TEST_F(ProtocolTest, TrackManConnectionTest) {
    auto protocol = std::make_unique<TrackManProtocolImpl>();
    EXPECT_TRUE(protocol->connect("127.0.0.1", 12345));
    EXPECT_TRUE(protocol->isConnected());
    EXPECT_TRUE(protocol->disconnect());
    EXPECT_FALSE(protocol->isConnected());
}

TEST_F(ProtocolTest, TrackManCommandTest) {
    auto protocol = std::make_unique<TrackManProtocolImpl>();
    EXPECT_TRUE(protocol->connect("127.0.0.1", 12345));
    
    // Test initialization command
    std::vector<uint8_t> initParams = {0x01, 0x00};
    EXPECT_TRUE(protocol->sendCommand(TrackManProtocol::CommandType::INITIALIZE, initParams));
    
    // Test start tracking command
    EXPECT_TRUE(protocol->sendCommand(TrackManProtocol::CommandType::START_TRACKING));
    
    // Test stop tracking command
    EXPECT_TRUE(protocol->sendCommand(TrackManProtocol::CommandType::STOP_TRACKING));
    
    EXPECT_TRUE(protocol->disconnect());
}

TEST_F(ProtocolTest, TrackManDataParsingTest) {
    auto protocol = std::make_unique<TrackManProtocolImpl>();
    EXPECT_TRUE(protocol->connect("127.0.0.1", 12345));
    
    // Send mock shot data
    std::string mockData = R"({
        "ball": {
            "speed": 70.5,
            "launch_angle": 12.3,
            "total_spin": 2800,
            "launch_direction": -1.2,
            "carry": 245.6,
            "total": 268.4
        },
        "advanced": {
            "smash_factor": 1.46,
            "spin_axis": -2.5,
            "apex": 32.1,
            "descent_angle": 42.3
        }
    })";
    
    server_->sendShotData(mockData);
    
    auto packet = protocol->receivePacket();
    ASSERT_TRUE(packet.has_value());
    
    LaunchMonitorData data;
    EXPECT_TRUE(protocol->parseShot(*packet, data));
    
    EXPECT_NEAR(data.ballSpeed, 70.5, 0.1);
    EXPECT_NEAR(data.launchAngle, 12.3, 0.1);
    EXPECT_NEAR(data.spinRate, 2800.0, 0.1);
    EXPECT_NEAR(data.launchDirection, -1.2, 0.1);
    EXPECT_NEAR(data.carryDistance, 245.6, 0.1);
    EXPECT_NEAR(data.totalDistance, 268.4, 0.1);
    
    EXPECT_TRUE(protocol->disconnect());
}

TEST_F(ProtocolTest, GCQuadConnectionTest) {
    auto protocol = std::make_unique<GCQuadProtocolImpl>();
    EXPECT_TRUE(protocol->connect("127.0.0.1", 12345));
    EXPECT_TRUE(protocol->isConnected());
    
    // Test camera initialization
    EXPECT_TRUE(protocol->calibrateCameras());
    
    // Test streaming
    EXPECT_TRUE(protocol->startStreaming(10000));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto frames = protocol->captureFrames();
    EXPECT_FALSE(frames.empty());
    
    EXPECT_TRUE(protocol->stopStreaming());
    EXPECT_TRUE(protocol->disconnect());
    EXPECT_FALSE(protocol->isConnected());
}

TEST_F(ProtocolTest, GCQuadCameraConfigTest) {
    auto protocol = std::make_unique<GCQuadProtocolImpl>();
    EXPECT_TRUE(protocol->connect("127.0.0.1", 12345));
    
    // Test quadruplex mode
    EXPECT_TRUE(protocol->setCameraMode(true));
    EXPECT_TRUE(protocol->startStreaming(10000));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto quadFrames = protocol->captureFrames();
    EXPECT_EQ(quadFrames.size(), 4); // Should get frames from all 4 cameras
    
    EXPECT_TRUE(protocol->stopStreaming());
    
    // Test dual camera mode
    EXPECT_TRUE(protocol->setCameraMode(false));
    EXPECT_TRUE(protocol->startStreaming(10000));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto dualFrames = protocol->captureFrames();
    EXPECT_EQ(dualFrames.size(), 2); // Should get frames from 2 cameras
    
    EXPECT_TRUE(protocol->stopStreaming());
    EXPECT_TRUE(protocol->disconnect());
}

TEST_F(ProtocolTest, TimeoutHandlingTest) {
    auto protocol = std::make_unique<TrackManProtocolImpl>();
    protocol->setTimeout(std::chrono::milliseconds(100));
    
    // Test connection timeout
    EXPECT_FALSE(protocol->connect("192.0.2.1", 12345)); // Non-existent address
    EXPECT_FALSE(protocol->isConnected());
    
    // Test read timeout
    EXPECT_TRUE(protocol->connect("127.0.0.1", 12345));
    auto result = protocol->receivePacket(); // Should timeout
    EXPECT_FALSE(result.has_value());
}

TEST_F(ProtocolTest, ErrorHandlingTest) {
    auto protocol = std::make_unique<GCQuadProtocolImpl>();
    EXPECT_TRUE(protocol->connect("127.0.0.1", 12345));
    
    // Test invalid camera mode
    EXPECT_FALSE(protocol->startStreaming(1000000)); // Invalid frame rate
    
    // Test calibration failure handling
    server_->sendShotData("{\"status\":\"error\",\"message\":\"calibration failed\"}");
    EXPECT_FALSE(protocol->calibrateCameras());
    
    EXPECT_TRUE(protocol->disconnect());
}
