# P-touch ESP32 Testing Framework

## Overview

This testing framework provides comprehensive validation of the P-touch ESP32 codebase with **two complementary approaches**:

1. **Native Mock Testing** (`test/` directory): Tests run on any development machine without hardware
2. **🆕 ESP32-S3 Hardware Testing** (`test/hardware/` directory): Tests run directly on real ESP32-S3 hardware

## Testing Strategies

### Native Mock Testing (Existing)
- ✅ **Fast Development**: Instant feedback during development
- ✅ **CI/CD Friendly**: No hardware required for automation
- ✅ **Comprehensive Mocking**: Simulates USB Host, WiFi, and hardware APIs
- ✅ **Protocol Validation**: Tests Brother P-touch protocol logic

### 🆕 ESP32-S3 Hardware Testing (New)
- ✅ **Real Hardware Validation**: Tests actual ESP-IDF USB Host API calls
- ✅ **Progressive Testing**: Bridge between mocks and full P-touch integration  
- ✅ **Zero P-touch Dependency**: Validates core functionality without printers
- ✅ **Performance Testing**: Real timing, memory usage, and system limits

## Design Principles

1. **Complete Separation**: Zero impact on production code
2. **Hardware Independence**: Native tests run on any development machine
3. **Progressive Validation**: Mock → Hardware → Full Integration
4. **Comprehensive Coverage**: Unit, integration, and protocol-level tests
5. **CI/CD Ready**: Automated testing in GitHub Actions
6. **Easy Maintenance**: Clear structure for adding new tests

## Testing Architecture

```
test/
├── README.md                 # This file - testing documentation
├── CMakeLists.txt            # Native test build configuration
├── runner/                   # Test execution framework
│   ├── main.cpp             # Native test runner entry point
│   ├── test_runner.h        # Test framework utilities
│   └── config.h             # Test configuration
├── hardware/                 # 🆕 ESP32-S3 hardware testing framework
│   ├── README.md            # Hardware testing documentation
│   ├── QUICK_START.md       # 5-minute setup guide
│   ├── platformio.ini       # PlatformIO build configuration
│   ├── CMakeLists.txt       # ESP-IDF build configuration
│   ├── hardware_test_runner.h/.cpp  # Hardware test framework
│   ├── test_usb_host_hardware.cpp   # Real USB Host API tests
│   ├── test_core_systems.cpp        # ESP32-S3 core functionality
│   ├── test_wifi_hardware.cpp       # WiFi connectivity tests
│   └── test_integration_hardware.cpp # Hardware integration tests
├── mocks/                    # Hardware abstraction layer
│   ├── mock_esp_system.h/.cpp     # ESP-IDF system mocks
│   ├── mock_usb_host.h/.cpp       # USB Host API simulation  
│   ├── mock_wifi.h/.cpp           # WiFi API simulation
│   ├── mock_http_server.h/.cpp    # HTTP server simulation
│   └── hardware_abstraction.h     # Common mock interface
├── unit/                     # Individual component tests
│   ├── test_protocol_parsing.cpp  # Brother P-touch protocol logic
│   ├── test_image_processing.cpp  # Image/raster operations
│   ├── test_printer_state.cpp     # Device state management
│   ├── test_web_endpoints.cpp     # HTTP API endpoints
│   └── test_usb_communication.cpp # USB transfer logic
├── integration/              # End-to-end workflow tests
│   ├── test_print_workflow.cpp    # Complete print job simulation
│   ├── test_error_handling.cpp    # Error recovery scenarios  
│   ├── test_connection_lifecycle.cpp # Connect/disconnect flows
│   └── test_concurrent_operations.cpp # Multi-threading safety
├── protocol/                 # Brother P-touch protocol validation
│   ├── test_command_generation.cpp    # Protocol command creation
│   ├── test_response_parsing.cpp      # Response interpretation
│   ├── known_sequences/             # Real printer data for validation
│   │   ├── pt_d460bt_init.bin      # Actual initialization sequences
│   │   ├── pt_p700_status.bin      # Real status responses
│   │   └── protocol_reference.md   # Documented command meanings
│   └── test_printer_models.cpp     # Model-specific behavior
├── fixtures/                 # Test data and utilities
│   ├── test_data.h          # Common test constants and data
│   ├── mock_responses.h     # Simulated printer responses
│   ├── test_images.h        # Sample images for testing
│   └── test_helpers.h       # Utility functions
└── coverage/                 # Code coverage reports
    └── .gitkeep
```

## Test Categories

### Unit Tests (`test/unit/`)
Test individual functions and classes in isolation:

- **Protocol Parsing**: Command generation, response parsing
- **Image Processing**: Rasterization, compression algorithms
- **State Management**: Printer connection state, error tracking
- **Web APIs**: HTTP endpoint logic, JSON handling
- **USB Communication**: Transfer logic, error handling

### Integration Tests (`test/integration/`)
Test component interactions and workflows:

- **Print Workflow**: End-to-end printing simulation
- **Error Handling**: USB disconnects, printer errors, recovery
- **Connection Lifecycle**: Detection, connection, disconnection
- **Concurrent Operations**: Thread safety, resource sharing

### Protocol Tests (`test/protocol/`)
Validate Brother P-touch protocol implementation:

- **Command Generation**: Verify correct protocol commands
- **Response Parsing**: Parse real printer responses correctly
- **Model Compatibility**: Test printer-specific behaviors
- **Edge Cases**: Malformed responses, timeout handling

## How Mocking Works

### Hardware Abstraction Strategy

Instead of mocking ESP-IDF functions directly, we create abstract interfaces:

```cpp
// production code uses interfaces
class IUSBHost {
public:
    virtual esp_err_t device_open(/*...*/) = 0;
    virtual esp_err_t transfer_submit(/*...*/) = 0;
    // ... other USB operations
};

// Production: Real ESP-IDF implementation
class ESPUSBHost : public IUSBHost { /*...*/ };

// Testing: Controllable mock implementation  
class MockUSBHost : public IUSBHost { /*...*/ };
```

### Mock Capabilities

- **Controllable Responses**: Set up expected USB responses
- **Error Injection**: Simulate USB errors, timeouts, disconnects
- **Sequence Validation**: Verify correct command sequences
- **Performance Simulation**: Model transfer delays, bandwidth

## Running Tests

### Native Mock Testing

#### Prerequisites

```bash
# Install CMake and a C++ compiler
# On macOS:
brew install cmake

# On Ubuntu:
sudo apt-get install cmake build-essential

# On Windows:
# Install Visual Studio or MinGW
```

#### Build and Run

```bash
# Build tests
cd test
mkdir build && cd build
cmake ..
make

# Run all tests
./ptouch_tests

# Run specific test categories
./ptouch_tests --unit-only
./ptouch_tests --integration-only  
./ptouch_tests --protocol-only

# Run with verbose output
./ptouch_tests --verbose

# Generate coverage report
make coverage
```

### 🆕 ESP32-S3 Hardware Testing

#### Quick Start (5 minutes)

```bash
# Navigate to hardware tests
cd test/hardware

# Connect ESP32-S3 board and flash all tests
pio run --target upload && pio device monitor

# Or run specific test categories
pio run -e esp32s3_usb_host_tests --target upload && pio device monitor
pio run -e esp32s3_core_tests --target upload && pio device monitor
```

**See [`test/hardware/QUICK_START.md`](hardware/QUICK_START.md) for detailed setup instructions.**

#### What Gets Tested
- **USB Host API**: Real ESP-IDF USB Host library functionality
- **Core Systems**: GPIO, memory, timing, FreeRTOS tasks
- **Performance**: Actual hardware benchmarks
- **Error Handling**: Real hardware error scenarios

### Continuous Integration

Tests run automatically on every commit via GitHub Actions:

```yaml
# .github/workflows/test.yml
name: Test Suite
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run Tests
        run: |
          cd test
          mkdir build && cd build
          cmake ..
          make
          ./ptouch_tests
```

## Test Framework Design

### Lightweight Test Framework

We use a custom lightweight framework instead of heavy dependencies:

```cpp
// test/runner/test_runner.h
#define TEST(name) \
    void test_##name(); \
    TestRegistry::instance().add_test(#name, test_##name); \
    void test_##name()

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        throw TestFailure(__FILE__, __LINE__, #expected " != " #actual); \
    }

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        throw TestFailure(__FILE__, __LINE__, #condition " is false"); \
    }

// Usage:
TEST(protocol_init_command) {
    MockUSBHost usb;
    PtouchPrinter printer(&usb);
    
    printer.initialize();
    
    auto commands = usb.getSentCommands();
    ASSERT_EQ(1, commands.size());
    ASSERT_EQ(0x1B, commands[0][0]); // ESC command
}
```

### Mock Framework

```cpp
// test/mocks/mock_usb_host.h
class MockUSBHost : public IUSBHost {
private:
    std::vector<std::vector<uint8_t>> sent_commands;
    std::queue<std::vector<uint8_t>> response_queue;
    
public:
    // Set up expected responses
    void queueResponse(const std::vector<uint8_t>& response);
    
    // Inject errors for testing  
    void setNextTransferError(esp_err_t error);
    
    // Verify sent commands
    const auto& getSentCommands() const { return sent_commands; }
    
    // IUSBHost implementation
    esp_err_t device_open(/*...*/) override;
    esp_err_t transfer_submit(/*...*/) override;
};
```

## Adding New Tests

### 1. Unit Test Example

```cpp
// test/unit/test_protocol_parsing.cpp
#include "../runner/test_runner.h"
#include "../mocks/mock_usb_host.h"
#include "../../components/ptouch-esp32/include/ptouch_esp32.h"

TEST(protocol_status_request) {
    MockUSBHost usb;
    // Queue a realistic status response
    usb.queueResponse({0x80, 0x20, 0x42, 0x30, /*... status data ...*/});
    
    PtouchPrinter printer(&usb);
    printer.connect();
    
    bool result = printer.getStatus();
    
    ASSERT_TRUE(result);
    ASSERT_EQ("PT-D460BT", printer.getPrinterName());
    ASSERT_EQ(24, printer.getTapeWidth());
}
```

### 2. Integration Test Example

```cpp
// test/integration/test_print_workflow.cpp
TEST(complete_print_workflow) {
    MockUSBHost usb;
    MockWiFi wifi;
    MockHTTPServer server;
    
    // Set up successful responses for entire workflow
    usb.queueResponse(status_response);
    usb.queueResponse(ack_response);
    
    // Simulate complete print job
    PtouchSystem system(&usb, &wifi, &server);
    system.initialize();
    
    auto result = system.printText("Hello World");
    
    ASSERT_TRUE(result.success);
    
    // Verify correct command sequence was sent
    auto commands = usb.getSentCommands();
    ASSERT_EQ(5, commands.size()); // init, status, raster start, data, finalize
}
```

### 3. Protocol Test Example

```cpp
// test/protocol/test_command_generation.cpp
TEST(d460bt_magic_command) {
    // Load known-good command from real printer capture
    auto expected = loadBinaryFile("known_sequences/pt_d460bt_magic.bin");
    
    PtouchProtocol protocol;
    auto actual = protocol.generateD460BTMagic();
    
    ASSERT_EQ(expected.size(), actual.size());
    ASSERT_EQ(expected, actual);
}
```

## Code Coverage

Tests aim for high code coverage while focusing on critical functionality:

- **Target**: 90%+ coverage of core logic
- **Priority**: Protocol logic, state management, error handling
- **Tool**: gcov/lcov for coverage analysis
- **Reports**: Generated automatically in CI

## Best Practices

### Writing Good Tests

1. **Test Behavior, Not Implementation**: Focus on what the code should do
2. **Use Realistic Data**: Base mocks on actual printer responses  
3. **Test Error Cases**: Don't just test the happy path
4. **Keep Tests Independent**: No shared state between tests
5. **Clear Test Names**: Describe what is being tested

### Mock Guidelines

1. **Minimal Mocking**: Only mock external dependencies
2. **Realistic Responses**: Use data from real printers when possible
3. **Error Injection**: Test how code handles failures
4. **Verification**: Check that correct calls were made

### Maintenance

1. **Update Tests with Code**: Keep tests in sync with implementation changes
2. **Add Tests for Bugs**: Every bug fix should include a test
3. **Review Coverage**: Regularly check coverage reports
4. **Clean Up**: Remove obsolete tests, refactor duplicated code

## Troubleshooting

### Common Issues

**Build Failures:**
- Check CMake version (3.16+ required)
- Verify C++17 support
- Check include paths in CMakeLists.txt

**Test Failures:**
- Run with `--verbose` for detailed output
- Check mock setup - ensure responses are queued
- Verify test data files are present

**Coverage Issues:**
- Ensure debug symbols are enabled (`-g -O0`)
- Check that gcov is installed
- Verify coverage tool configuration

### Getting Help

1. Check test output with `--verbose` flag
2. Review mock setup in failing tests
3. Validate test data against real printer responses
4. Check GitHub Actions logs for CI failures

This testing framework provides comprehensive validation while maintaining clean separation from production code and enabling fast development cycles without hardware dependencies. 