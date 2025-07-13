# P-touch ESP32 Test Suite Documentation

## Overview

The P-touch ESP32 test suite provides comprehensive testing for the ESP32-based Brother P-touch printer interface without requiring physical hardware. It uses a custom lightweight testing framework with mock hardware abstractions to simulate USB Host operations, WiFi connectivity, and web server functionality.

## Architecture

### Test Framework Structure

```
test/
├── runner/           # Custom test runner implementation
├── mocks/           # Hardware abstraction mocks
├── fixtures/        # Test data and helper utilities
├── unit/            # Unit tests for individual components
├── integration/     # Integration tests for workflows
├── protocol/        # Brother P-touch protocol tests
└── build/           # Build artifacts
```

### Core Components

1. **Test Runner** (`runner/test_runner.h`)
   - Lightweight custom framework with macros (`TEST`, `ASSERT_EQ`, `ASSERT_TRUE`, etc.)
   - Automatic test registration and execution
   - Configurable output verbosity and filtering
   - Statistics tracking and reporting

2. **Mock Hardware Layer** (`mocks/`)
   - `MockUSBHost`: Simulates ESP32 USB Host API
   - `IUSBHost`: Abstract interface for USB operations
   - `IWiFiManager`, `IHTTPServer`: WiFi and web server abstractions

3. **Test Fixtures** (`fixtures/`)
   - `test_data.h`: Brother P-touch protocol constants and commands
   - `test_helpers.h`: Utility functions and vector comparison helpers
   - `mock_responses.h`: Simulated printer responses

## Building and Running Tests

### Prerequisites

- CMake 3.10+
- C++17 compatible compiler (GCC/Clang)
- Make build system

### Build Process

```bash
cd test
mkdir -p build
cd build
cmake ..
make
```

### Running Tests

#### All Tests
```bash
./ptouch_tests
```

#### Verbose Output
```bash
./ptouch_tests --verbose
```

#### Category-Specific Tests
```bash
./ptouch_tests --unit-only           # Unit tests only
./ptouch_tests --integration-only    # Integration tests only
./ptouch_tests --protocol-only       # Protocol tests only
```

#### List Available Tests
```bash
./ptouch_tests --list
```

### Expected Outputs

#### Successful Run
```
P-touch ESP32 Test Suite
========================
Configuration:
  Verbose: No
  Filter: All tests

Running P-touch ESP32 Test Suite
=================================
......................

Results:
  Passed: 22
  Failed: 0
  Total:  22
  Time:   0ms

ALL TESTS PASSED!
```

#### Verbose Output
```
Running unit::USBHostDriverInstallation... PASS
Running unit::USBBulkTransferOut... PASS
Running unit::USBErrorInjection... PASS
...
```

#### Test Failure Example
```
Running unit::SomeTest... FAIL
  File: test/unit/test_example.cpp:42
  Expected: 5
  Actual: 3
  Message: Value mismatch in transfer count
```

## Test Categories

### Unit Tests (15 tests)

**USB Host Driver Tests:**
- `USBHostDriverInstallation`: Driver installation/uninstallation
- `USBHostDriverUninstallation`: Error handling for double operations
- `USBClientRegistration`: Client registration lifecycle

**USB Device Management Tests:**
- `PtouchPrinterDiscovery`: Device enumeration and identification
- `USBDeviceOpenClose`: Device handle management
- `USBDeviceDescriptor`: VID/PID reading and validation
- `USBInterfaceClaiming`: Interface claiming/releasing

**USB Communication Tests:**
- `USBBulkTransferOut`: Successful data transfer verification
- `USBErrorInjection`: Timeout and error simulation
- `MultipleDeviceManagement`: Concurrent device handling
- `USBTransferProtocolLogging`: Brother P-touch command logging

**Placeholder Tests:**
- `protocol_parsing_placeholder`: Protocol command parsing
- `image_processing_placeholder`: Image data processing
- `printer_state_placeholder`: Printer status management
- `web_endpoints_placeholder`: Web API endpoints

### Integration Tests (4 tests)

**Workflow Tests:**
- `print_workflow_placeholder`: End-to-end printing workflow
- `error_handling_placeholder`: Error recovery scenarios
- `connection_lifecycle_placeholder`: Device connection management
- `concurrent_operations_placeholder`: Multi-device operations

### Protocol Tests (3 tests)

**Brother P-touch Protocol:**
- `command_generation_placeholder`: Command creation and validation
- `response_parsing_placeholder`: Printer response interpretation
- `printer_models_placeholder`: Multi-model compatibility

## Mock Hardware Capabilities

### MockUSBHost Features

**Device Simulation:**
- Add/remove Brother P-touch printers with specific PIDs
- Simulate USB device enumeration and management
- Track device handles and interface states

**Transfer Simulation:**
- Record all USB bulk transfers (IN/OUT)
- Queue custom responses for IN transfers
- Simulate transfer delays and timeouts

**Error Injection:**
- Inject specific error codes (ESP_ERR_TIMEOUT, ESP_ERR_NO_MEM, etc.)
- Simulate hardware failures and recovery scenarios
- Track failure statistics and success rates

**Protocol Support:**
- Brother P-touch command sequences (INIT, STATUS, PACKBITS)
- Realistic printer status responses
- Transfer logging with timestamps

### Example Mock Usage

```cpp
MockUSBHost usb;
usb.install_host_driver();
usb.register_client();

// Add simulated P-touch printer
uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);

// Inject error for testing
usb.set_next_transfer_error(ESP_ERR_TIMEOUT);

// Queue realistic printer response
usb.queue_status_response(12, 1); // 12mm laminated tape

// Verify transfer logging
auto transfers = usb.get_sent_transfers();
ASSERT_EQ(expected_data, transfers[0].data);
```

## Current Limitations & Blind Spots

### Hardware Limitations

1. **No Real Hardware Testing**
   - Cannot test actual USB communication timing
   - No validation of electrical characteristics
   - Missing real printer response variations

2. **USB Host API Simulation**
   - Simplified device enumeration (no descriptor parsing)
   - No USB endpoint configuration testing
   - Missing USB suspend/resume functionality

3. **Memory Constraints**
   - No ESP32-specific memory allocation testing
   - Missing PSRAM/SPIRAM considerations
   - No memory fragmentation simulation

### Protocol Limitations

1. **Brother P-touch Protocol**
   - Limited printer model coverage (only 3 models)
   - Missing advanced features (cutting, tape detection)
   - No compression algorithm validation

2. **Image Processing**
   - No actual image conversion testing
   - Missing dithering algorithm validation
   - No print quality optimization testing

3. **Error Scenarios**
   - Limited error injection patterns
   - Missing complex failure cascades
   - No network-related error simulation

### Software Architecture

1. **WiFi/Network Stack**
   - No actual network connectivity testing
   - Missing WiFi configuration validation
   - No web server load testing

2. **Concurrency**
   - No multi-threaded operation testing
   - Missing FreeRTOS task simulation
   - No interrupt handler testing

3. **Storage/Configuration**
   - No SPIFFS/LittleFS testing
   - Missing configuration persistence
   - No OTA update simulation

## Future Work & Expansion

### Immediate Priorities

1. **Complete Protocol Tests**
   ```cpp
   // Implement real protocol parsing
   TEST(BrotherProtocolParsing) {
       auto cmd = ProtocolParser::parse_init_command();
       ASSERT_EQ(expected_bytes, cmd.to_bytes());
   }
   ```

2. **Add Image Processing Tests**
   ```cpp
   TEST(ImageDithering) {
       auto image = load_test_image("test_pattern.bmp");
       auto dithered = ImageProcessor::dither(image, 12); // 12mm tape
       ASSERT_EQ(expected_width, dithered.width);
   }
   ```

3. **Implement Web API Tests**
   ```cpp
   TEST(WebAPIEndpoints) {
       MockHTTPServer server;
       auto response = server.post("/print", json_data);
       ASSERT_EQ(200, response.status_code);
   }
   ```

### Medium-term Enhancements

1. **Hardware-in-the-Loop Testing**
   - Integration with real ESP32 hardware
   - Automated test runner on device
   - Real printer communication validation

2. **Performance Testing**
   - Memory usage profiling
   - Transfer speed benchmarks
   - Power consumption simulation

3. **Fuzzing and Robustness**
   - Malformed protocol command testing
   - Random data injection
   - Stress testing with rapid operations

### Long-term Vision

1. **Continuous Integration**
   - Automated testing on code changes
   - Cross-platform compatibility testing
   - Performance regression detection

2. **Test Coverage Analysis**
   - Code coverage reporting
   - Branch coverage validation
   - Dead code detection

3. **Documentation Testing**
   - Example code validation
   - API documentation accuracy
   - User guide testing

## Test Data Management

### Brother P-touch Constants

The test suite includes realistic Brother P-touch protocol data:

```cpp
// USB identifiers
constexpr uint16_t BROTHER_VID = 0x04F9;
constexpr uint16_t PT_P700_PID = 0x2061;

// Protocol commands
const std::vector<uint8_t> INIT_COMMAND = {0x1B, 0x40}; // ESC @
const std::vector<uint8_t> STATUS_REQUEST = {0x1B, 0x69, 0x53}; // ESC i S

// Status responses
const std::vector<uint8_t> BASIC_STATUS_RESPONSE = {
    0x80, 0x20, 0x42, 0x30,  // Header
    0x01, 0x00, 0x00, 0x00,  // Model info
    0x00, 0x00,              // No errors
    0x0C,                    // 12mm tape width
    0x01,                    // Laminated tape
    // ... additional fields
};
```

### Test Helpers

Utility functions for common test operations:

```cpp
// Vector comparison with hex output
bool vectors_equal(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);

// Hex string conversion for debugging
std::string bytes_to_hex(const std::vector<uint8_t>& bytes);

// Stream operator for test output
std::ostream& operator<<(std::ostream& os, const std::vector<uint8_t>& vec);
```

## Debugging and Troubleshooting

### Common Issues

1. **Build Failures**
   - Ensure CMake 3.10+ is installed
   - Check C++17 compiler support
   - Verify include paths are correct

2. **Test Failures**
   - Use `--verbose` flag for detailed output
   - Check test data constants match expectations
   - Verify mock state is properly reset between tests

3. **Memory Issues**
   - Mock objects are automatically cleaned up
   - Use `usb.reset()` for explicit cleanup
   - Check for memory leaks in custom test code

### Adding New Tests

1. **Create Test File**
   ```cpp
   #include "../runner/test_runner.h"
   #include "../mocks/mock_usb_host.h"
   
   TEST(YourTestName) {
       // Test implementation
       ASSERT_EQ(expected, actual);
   }
   ```

2. **Update CMakeLists.txt**
   ```cmake
   # Add to test sources
   set(TEST_SOURCES
       # ... existing sources
       unit/test_your_feature.cpp
   )
   ```

3. **Build and Run**
   ```bash
   make
   ./ptouch_tests --verbose
   ```

This test suite provides a solid foundation for validating the P-touch ESP32 implementation while highlighting areas for future development and improvement. 