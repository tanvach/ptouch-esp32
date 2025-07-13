#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <vector>
#include <cstdint>
#include <string>

// Brother P-touch constants for testing
namespace TestData {

    // USB Vendor/Product IDs
    constexpr uint16_t BROTHER_VID = 0x04F9;
    
    // Popular Brother P-touch models for testing
    constexpr uint16_t PT_D460BT_PID = 0x20e0;
    constexpr uint16_t PT_P700_PID = 0x2061;
    constexpr uint16_t PT_H500_PID = 0x205e;
    
    // Protocol constants
    constexpr uint8_t ESC = 0x1B;
    constexpr uint8_t ACK = 0x06;
    constexpr uint8_t PRINTHEADMARK = 0x80;
    constexpr uint8_t STATUS_SIZE = 0x20;
    
    // Status response template
    const std::vector<uint8_t> BASIC_STATUS_RESPONSE = {
        0x80, 0x20, 0x42, 0x30,  // printheadmark, size, "B", "0"
        0x01, 0x00, 0x00, 0x00,  // model, country, reserved
        0x00, 0x00,              // error (no error)
        0x0C,                    // media width (12mm)
        0x01,                    // media type (laminated)
        0x00, 0x00, 0x00, 0x00,  // ncol, fonts, jp_fonts, mode
        0x00, 0x00,              // density, media_len
        0x00, 0x00, 0x00, 0x00,  // status_type, phase_type, phase_number
        0x00, 0x00,              // notif_number, exp
        0x01, 0x00,              // tape_color (white), text_color (black)
        0x00, 0x00, 0x00, 0x00,  // hw_setting
        0x00, 0x00               // reserved
    };
    
    // Common protocol commands
    const std::vector<uint8_t> INIT_COMMAND = {ESC, 0x40}; // ESC @
    const std::vector<uint8_t> STATUS_REQUEST = {ESC, 0x69, 0x53}; // ESC i S
    const std::vector<uint8_t> PACKBITS_ENABLE = {0x4d, 0x02}; // Enable compression
    const std::vector<uint8_t> PRINT_FINALIZE = {0x1a}; // Print with feed
    
    // Invalidate command (100 zeros + ESC @)
    const std::vector<uint8_t> INVALIDATE_COMMAND = []() {
        std::vector<uint8_t> cmd(102);
        std::fill(cmd.begin(), cmd.begin() + 100, 0x00);
        cmd[100] = ESC;
        cmd[101] = 0x40;
        return cmd;
    }();
    
    // Sample raster line (8 bytes = 64 pixels)
    const std::vector<uint8_t> SAMPLE_RASTER_LINE = {
        0x47, 0x08,              // Raster line command + length
        0xFF, 0x00, 0xFF, 0x00,  // Alternating black/white pattern
        0xFF, 0x00, 0xFF, 0x00
    };
    
    // Error responses
    const std::vector<uint8_t> ERROR_STATUS_RESPONSE = []() {
        auto status = BASIC_STATUS_RESPONSE;
        status[8] = 0x01;  // Set error bit
        status[9] = 0x00;
        return status;
    }();
    
    // Different tape widths for testing
    struct TapeInfo {
        uint8_t width_mm;
        uint16_t width_px;
        const char* description;
    };
    
    const TapeInfo SUPPORTED_TAPES[] = {
        {6,  32,  "6mm tape"},
        {9,  52,  "9mm tape"},
        {12, 76,  "12mm tape"},
        {18, 120, "18mm tape"},
        {24, 128, "24mm tape"},
        {36, 192, "36mm tape"},
        {0,  0,   nullptr}  // Terminator
    };
    
    // Test text for printing
    const std::vector<std::string> TEST_TEXTS = {
        "Hello World",
        "ESP32 Test",
        "P-touch Printer",
        "1234567890",
        "!@#$%^&*()",
        "Mixed 123 Text!",
        "", // Empty string test
        "Very Long Text That Might Exceed Normal Limits For Testing Purposes"
    };
    
    // Mock printer names
    const std::vector<std::string> PRINTER_NAMES = {
        "PT-D460BT",
        "PT-P700", 
        "PT-H500",
        "PT-E500",
        "PT-9700PC"
    };
    
    // USB endpoints commonly used by P-touch printers
    constexpr uint8_t BULK_OUT_ENDPOINT = 0x02;
    constexpr uint8_t BULK_IN_ENDPOINT = 0x81;
    constexpr uint8_t INTERRUPT_ENDPOINT = 0x83;
    
    // Transfer timeouts
    constexpr uint32_t DEFAULT_TIMEOUT_MS = 1000;
    constexpr uint32_t LONG_TIMEOUT_MS = 5000;
    constexpr uint32_t SHORT_TIMEOUT_MS = 100;
    
    // HTTP API test data
    namespace API {
        const std::string STATUS_ENDPOINT = "/api/status";
        const std::string PRINT_TEXT_ENDPOINT = "/api/print/text";
        const std::string RECONNECT_ENDPOINT = "/api/reconnect";
        const std::string PRINTERS_ENDPOINT = "/api/printers";
        
        const std::string SAMPLE_PRINT_REQUEST = R"({
            "text": "Test Label",
            "margin": 3,
            "copies": 1
        })";
        
        const std::string EXPECTED_STATUS_RESPONSE = R"({
            "connected": true,
            "name": "PT-D460BT",
            "status": "Connected",
            "maxWidth": 128,
            "tapeWidth": 76,
            "mediaType": "Laminated",
            "tapeColor": "White",
            "textColor": "Black",
            "hasError": false
        })";
    }
    
    // WiFi test data
    namespace WiFi {
        const std::string TEST_SSID = "TestNetwork";
        const std::string TEST_PASSWORD = "testpassword123";
        const std::string TEST_IP = "192.168.1.100";
        const uint16_t TEST_PORT = 80;
    }
    
    // Memory allocation sizes for testing
    namespace Memory {
        constexpr size_t SMALL_ALLOC = 64;
        constexpr size_t MEDIUM_ALLOC = 1024;
        constexpr size_t LARGE_ALLOC = 4096;
        constexpr size_t HUGE_ALLOC = 65536;
    }
    
    // Timing constants for tests
    namespace Timing {
        constexpr uint32_t FAST_OPERATION_MS = 10;
        constexpr uint32_t NORMAL_OPERATION_MS = 100;
        constexpr uint32_t SLOW_OPERATION_MS = 1000;
        constexpr uint32_t VERY_SLOW_OPERATION_MS = 5000;
    }
    
    // Error codes for testing
    namespace Errors {
        constexpr int USB_TIMEOUT = -1;
        constexpr int USB_DISCONNECTED = -2;
        constexpr int PRINTER_ERROR = -3;
        constexpr int INVALID_COMMAND = -4;
        constexpr int OUT_OF_MEMORY = -5;
    }
}

#endif // TEST_DATA_H 