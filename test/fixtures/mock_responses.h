#ifndef MOCK_RESPONSES_H
#define MOCK_RESPONSES_H

#include <vector>
#include <cstdint>
#include <string>

// Mock response data for testing
namespace MockResponses {

    // Brother P-touch status responses for different models
    namespace StatusResponses {
        
        // PT-D460BT status response
        const std::vector<uint8_t> PT_D460BT_CONNECTED = {
            0x80, 0x20, 0x42, 0x30,  // Header
            0x01, 0x00, 0x00, 0x00,  // Model, country
            0x00, 0x00,              // No error
            0x0C,                    // 12mm tape
            0x01,                    // Laminated
            0x00, 0x00, 0x00, 0x00,  // Other fields
            0x00, 0x00, 0x00, 0x00,  // Status info
            0x00, 0x00, 0x00, 0x00,  // Phase info
            0x01, 0x00,              // White tape, black text
            0x00, 0x00, 0x00, 0x00,  // HW settings
            0x00, 0x00               // Reserved
        };
        
        // PT-P700 status response
        const std::vector<uint8_t> PT_P700_CONNECTED = {
            0x80, 0x20, 0x42, 0x30,  // Header
            0x02, 0x00, 0x00, 0x00,  // Different model
            0x00, 0x00,              // No error
            0x0C,                    // 12mm tape
            0x01,                    // Laminated
            0x00, 0x00, 0x00, 0x00,  // Other fields
            0x00, 0x00, 0x00, 0x00,  // Status info
            0x00, 0x00, 0x00, 0x00,  // Phase info
            0x01, 0x00,              // White tape, black text
            0x00, 0x00, 0x00, 0x00,  // HW settings
            0x00, 0x00               // Reserved
        };
        
        // Error status (paper jam)
        const std::vector<uint8_t> PAPER_JAM_ERROR = {
            0x80, 0x20, 0x42, 0x30,  // Header
            0x01, 0x00, 0x00, 0x00,  // Model, country
            0x02, 0x00,              // Paper jam error
            0x0C,                    // 12mm tape
            0x01,                    // Laminated
            0x00, 0x00, 0x00, 0x00,  // Other fields
            0x00, 0x00, 0x00, 0x00,  // Status info
            0x00, 0x00, 0x00, 0x00,  // Phase info
            0x01, 0x00,              // White tape, black text
            0x00, 0x00, 0x00, 0x00,  // HW settings
            0x00, 0x00               // Reserved
        };
        
        // No tape error
        const std::vector<uint8_t> NO_TAPE_ERROR = {
            0x80, 0x20, 0x42, 0x30,  // Header
            0x01, 0x00, 0x00, 0x00,  // Model, country
            0x01, 0x00,              // No tape error
            0x00,                    // No tape width
            0x00,                    // No media type
            0x00, 0x00, 0x00, 0x00,  // Other fields
            0x00, 0x00, 0x00, 0x00,  // Status info
            0x00, 0x00, 0x00, 0x00,  // Phase info
            0x00, 0x00,              // No colors
            0x00, 0x00, 0x00, 0x00,  // HW settings
            0x00, 0x00               // Reserved
        };
    }

    // Simple ACK/NACK responses
    const std::vector<uint8_t> ACK = {0x06};
    const std::vector<uint8_t> NACK = {0x15};
    
    // Empty response (timeout simulation)
    const std::vector<uint8_t> TIMEOUT = {};
    
    // HTTP API responses
    namespace HTTPResponses {
        
        const std::string PRINTER_CONNECTED_STATUS = R"({
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
        
        const std::string PRINTER_DISCONNECTED_STATUS = R"({
            "connected": false,
            "name": "Unknown",
            "status": "Not detected",
            "maxWidth": 0,
            "tapeWidth": 0,
            "mediaType": "-",
            "tapeColor": "-",
            "textColor": "-",
            "hasError": false
        })";
        
        const std::string PRINTER_ERROR_STATUS = R"({
            "connected": true,
            "name": "PT-D460BT",
            "status": "Connected",
            "maxWidth": 128,
            "tapeWidth": 76,
            "mediaType": "Laminated",
            "tapeColor": "White",
            "textColor": "Black",
            "hasError": true,
            "errorDescription": "Paper jam detected"
        })";
        
        const std::string PRINT_SUCCESS = R"({
            "success": true,
            "message": "Print job sent successfully"
        })";
        
        const std::string PRINT_FAILURE = R"({
            "success": false,
            "message": "Printer not connected"
        })";
        
        const std::string SUPPORTED_PRINTERS = R"({
            "printers": [
                {
                    "name": "PT-D460BT",
                    "vid": 1273,
                    "pid": 8416,
                    "maxWidth": 128,
                    "dpi": 180
                },
                {
                    "name": "PT-P700",
                    "vid": 1273,
                    "pid": 8289,
                    "maxWidth": 128,
                    "dpi": 180
                }
            ]
        })";
    }
}

#endif // MOCK_RESPONSES_H 