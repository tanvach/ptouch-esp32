# ESP32-S3 Hardware Testing Framework

## Overview

This hardware testing framework extends the existing P-touch ESP32 test suite to run **directly on ESP32-S3 hardware**. It provides comprehensive validation of USB Host functionality, core systems, and integration scenarios without requiring external P-touch printers.

## Why Hardware Testing?

The hardware testing framework bridges the gap between mock-based testing and full integration testing:

- ✅ **Real Hardware Validation**: Test actual ESP-IDF USB Host API calls
- ✅ **Progressive Testing**: Mock tests → Hardware tests → P-touch integration
- ✅ **Zero P-touch Dependency**: Validate core functionality without printers
- ✅ **Performance Testing**: Real timing, memory usage, and system limits
- ✅ **CI/CD Ready**: Automated testing with real hardware in CI pipeline

## Hardware Requirements

### Minimum Requirements
- **ESP32-S3 development board** (ESP32-S3-DevKitC-1 or similar)
- **USB-C cable** for programming and power
- **Computer** with ESP-IDF and PlatformIO installed
- **⚠️ NO USB CLIENT DEVICES REQUIRED** - Tests run without external USB devices

### USB Device Connection Requirements

**Current Tests (v1.0)**: **NO USB client devices needed**
- ✅ Tests validate USB Host API functionality only
- ✅ Device enumeration tests with no devices connected
- ✅ Transfer allocation/deallocation without actual transfers
- ✅ Perfect for initial validation and development

**Future Tests (v2.0+)**: May require USB test devices
- USB flash drives for actual device communication
- P-touch printers for full integration testing

### Recommended Setup (for Future Extensions)
- **ESP32-S3-USB-OTG** development board (for USB Host testing)
- **USB devices** for testing (flash drives, mice, keyboards)
- **WiFi access point** for network testing
- **Logic analyzer** (optional, for advanced debugging)

## Software Prerequisites

### ESP-IDF Installation
```bash
# Install ESP-IDF (v5.0 or later recommended)
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
source export.sh
```

### PlatformIO Installation (Alternative)
```bash
# Install PlatformIO CLI
pip install platformio

# Or use VS Code with PlatformIO extension
```

## Building and Running Tests

### Option 1: Using PlatformIO (Recommended)

```bash
# Navigate to hardware test directory
cd test/hardware

# Build and flash all tests
pio run --target upload --environment esp32s3_hardware_tests

# Monitor output
pio device monitor --baud 115200

# Or do both in one command
pio run --target upload && pio device monitor
```

### Option 2: Using ESP-IDF

```bash
# Navigate to hardware test directory
cd test/hardware

# Set target and configure
idf.py set-target esp32s3
idf.py menuconfig

# Build and flash
idf.py build flash monitor
```

## Test Configurations

The framework provides multiple build configurations for different testing scenarios:

### All Tests (Default)
```bash
pio run -e esp32s3_hardware_tests --target upload
```

### USB Host Tests Only
```bash
pio run -e esp32s3_usb_host_tests --target upload
```

### Core Systems Tests Only
```bash
pio run -e esp32s3_core_tests --target upload
```

### WiFi Tests Only
```bash
pio run -e esp32s3_wifi_tests --target upload
```

### Performance Tests (Release Build)
```bash
pio run -e esp32s3_performance_tests --target upload
```

### ESP32-S3-USB-OTG Board
```bash
pio run -e esp32s3_usb_otg_board --target upload
```

## Test Categories

### USB Host Tests (`usb_host` category)
- **USB Host Library Lifecycle**: Installation, configuration, uninstallation
- **Client Registration**: USB client management
- **Port Power Management**: USB port power control
- **Device Enumeration**: Device detection (without devices connected)
- **Transfer Allocation**: USB transfer buffer management
- **Timing Performance**: USB operation performance benchmarks
- **Memory Usage**: Memory consumption monitoring
- **Error Handling**: Error injection and recovery

### Core Systems Tests (`hardware` category)
- **GPIO Operations**: Digital I/O functionality
- **Timing Accuracy**: FreeRTOS delay and timer accuracy
- **Memory Management**: Heap allocation and deallocation
- **Random Number Generation**: Hardware RNG functionality
- **FreeRTOS Tasks**: Task creation and management
- **CPU Performance**: Processing speed benchmarks
- **Watchdog Timer**: Watchdog functionality
- **Heap Capabilities**: Different memory types (DMA, SPIRAM, internal)
- **System Information**: Chip identification and versioning

### WiFi Tests (`wifi` category)
*Note: WiFi tests require configuration of access point credentials*

### Integration Tests (`integration` category)
- **Concurrent Operations**: Multi-system interaction testing
- **Error Recovery**: Complex failure scenarios
- **Resource Management**: System resource sharing

## ⚠️ Test Limitations and Reliability

### USB Host Tests
**WHAT THESE TESTS VERIFY:**
- ✅ ESP-IDF USB Host API functions work correctly
- ✅ USB Host library can be installed/uninstalled
- ✅ Client registration/deregistration works
- ✅ Transfer allocation/deallocation works
- ✅ Basic error handling behaves correctly

**WHAT THESE TESTS DO NOT VERIFY:**
- ❌ Actual USB device communication
- ❌ Real device enumeration with connected devices
- ❌ Data transfer operations
- ❌ Protocol-specific communication
- ❌ Hardware-level USB signaling
- ❌ Device compatibility issues

### Core Systems Tests
**WHAT THESE TESTS VERIFY:**
- ✅ Basic GPIO operations work
- ✅ Timer and delay functions work
- ✅ Memory allocation/deallocation works
- ✅ FreeRTOS tasks can be created
- ✅ Hardware random number generation works

**WHAT THESE TESTS DO NOT VERIFY:**
- ❌ Complex hardware interactions
- ❌ Real-world timing constraints
- ❌ Extended operation scenarios
- ❌ Hardware-specific quirks
- ❌ Stress testing under load

### ⚠️ IMPORTANT DISCLAIMER
These tests are designed to verify that basic ESP-IDF functionality works correctly on your ESP32-S3 hardware. They serve as a foundation for confidence in your development environment and basic hardware functionality.

**They are NOT a substitute for:**
- Real-world testing with actual P-touch printers
- Comprehensive integration testing
- Performance testing under load
- Long-term reliability testing
- Hardware stress testing

**Use these tests to:**
- Verify your ESP32-S3 hardware works
- Confirm ESP-IDF installation is correct
- Validate basic functionality before moving to integration testing
- Identify hardware or software configuration issues early

## Expected Test Output

### Successful Run
```
I (1234) HARDWARE_TEST: ESP32-S3 P-touch Hardware Test Suite Starting...
I (1235) HARDWARE_TEST: Free memory: 390000 bytes
I (1236) HARDWARE_TEST: IDF version: v5.1.2
I (1237) HARDWARE_TEST: Hardware test environment initialized
I (1238) HARDWARE_TEST: Found 15 hardware tests
I (1239) HARDWARE_TEST: =================================
I (1240) HARDWARE_TEST: ESP32-S3 Hardware Test Suite
I (1241) HARDWARE_TEST: =================================
I (1242) HARDWARE_TEST: Configuration:
I (1243) HARDWARE_TEST:   Verbose: Yes
I (1244) HARDWARE_TEST:   Filter: All tests
I (1245) HARDWARE_TEST:   Tests: 15
I (1246) HARDWARE_TEST: 
I (1247) HARDWARE_TEST: Running usb_host::host_library_lifecycle...
I (1248) USB_HOST_HW_TEST: Testing USB Host library lifecycle
I (1250) USB_HOST_HW_TEST: USB Host library lifecycle test passed
I (1251) HARDWARE_TEST:   PASS
...
I (5678) HARDWARE_TEST: Results:
I (5679) HARDWARE_TEST:   Passed: 15
I (5680) HARDWARE_TEST:   Failed: 0
I (5681) HARDWARE_TEST:   Total:  15
I (5682) HARDWARE_TEST:   Time:   4234 ms
I (5683) HARDWARE_TEST: 
I (5684) HARDWARE_TEST: ALL TESTS PASSED!
I (5685) HARDWARE_TEST: Hardware test suite completed successfully
```

### Test Failure Example
```
I (1500) HARDWARE_TEST: Running usb_host::memory_usage...
E (1501) HARDWARE_TEST:   FAIL: Test failure at test_usb_host_hardware.cpp:245 - Insufficient memory: 45000 < 50000
```

## Hardware-Specific Features

### LED Indicators
The framework uses the built-in LED (GPIO2) to indicate test status:
- **Solid flash on startup**: Tests starting
- **Three quick flashes**: Tests completed successfully
- **Continuous blinking**: Tests running (heartbeat)

### Memory Monitoring
Real-time memory usage tracking:
```cpp
HW_ASSERT_MEMORY_AVAILABLE(50000); // Require 50KB free
HW_ASSERT_TIMING(operation(), 1000); // Max 1 second
```

### Hardware-Specific Assertions
```cpp
HW_ASSERT_ESP_OK(esp_function());          // ESP-IDF error codes
HW_ASSERT_GPIO_LEVEL(GPIO_NUM_2, 1);       // GPIO state verification
HW_ASSERT_USB_DEVICE_CONNECTED(handle);    // USB device validation
HW_ASSERT_WIFI_CONNECTED();                // WiFi connectivity
```

## Configuration Options

### Compile-Time Flags
- `HARDWARE_TESTING`: Enable hardware testing mode
- `TEST_ON_DEVICE`: Device-specific optimizations
- `TEST_VERBOSE`: Detailed test output
- `ENABLE_USB_HOST_TESTS`: Include USB Host tests
- `ENABLE_WIFI_TESTS`: Include WiFi tests
- `PERFORMANCE_TESTING`: Optimize for performance benchmarks

### Runtime Configuration
Configure via ESP-IDF menuconfig:
```bash
idf.py menuconfig
# Navigate to Component config → P-touch ESP32 Hardware Tests
```

## Adding New Hardware Tests

### 1. Create Test Function
```cpp
// test/hardware/test_your_feature.cpp
#include "hardware_test_runner.h"

USB_HOST_TEST(your_usb_test) {
    ESP_LOGI("YOUR_TEST", "Testing your USB feature");
    
    // Your test implementation
    HW_ASSERT_ESP_OK(your_usb_function());
    
    ESP_LOGI("YOUR_TEST", "Test passed");
}

HARDWARE_TEST(your_core_test) {
    // Test core functionality
    HW_ASSERT_TRUE(your_condition);
}
```

### 2. Update Build Configuration
Add your test file to the build system:

**PlatformIO**: Tests are automatically included if they follow the naming pattern.

**ESP-IDF**: Add to `CMakeLists.txt`:
```cmake
set(HARDWARE_TEST_SOURCES
    # ... existing sources
    "test_your_feature.cpp"
)
```

### 3. Test Categories
Use appropriate test macros:
- `USB_HOST_TEST(name)` - USB Host functionality
- `WIFI_TEST(name)` - WiFi functionality  
- `HARDWARE_TEST(name)` - General hardware tests
- `INTEGRATION_HARDWARE_TEST(name)` - Integration tests

## Troubleshooting

### Common Issues

**Build Errors:**
```bash
# Clean build
pio run --target clean
# Or with ESP-IDF
idf.py fullclean
```

**Flash Errors:**
```bash
# Check USB connection and try different port
pio device list
# Reset board and try again
```

**Test Failures:**
- Check power supply (USB Host needs sufficient power)
- Verify board type matches configuration
- Check memory constraints (some tests need significant RAM)

**USB Host Tests Failing:**
- Ensure ESP32-S3 board supports USB Host (not all do)
- Check USB connector wiring
- Verify sufficient power supply

### Debug Output
Enable verbose logging:
```bash
# Build with verbose output
pio run -e esp32s3_all_tests_verbose --target upload

# Monitor with debug level
pio device monitor --baud 115200 --filter debug
```

### Memory Issues
Monitor memory usage:
```cpp
ESP_LOGI(TAG, "Free memory: %zu bytes", esp_get_free_heap_size());
```

## Performance Benchmarks

Expected performance on ESP32-S3 @ 240MHz:

- **USB Host Installation**: < 1000ms
- **USB Transfer Allocation (10x)**: < 500ms  
- **GPIO Operations**: < 1μs per operation
- **Memory Allocation (4KB)**: < 10ms
- **FreeRTOS Task Creation**: < 50ms

## CI/CD Integration

### GitHub Actions Example
```yaml
name: Hardware Tests
on: [push, pull_request]

jobs:
  hardware-test:
    runs-on: [self-hosted, esp32s3]
    steps:
      - uses: actions/checkout@v3
      - name: Run Hardware Tests
        run: |
          cd test/hardware
          pio run --target upload
          # Parse test results from serial output
```

### Test Result Parsing
Parse test output for CI integration:
```bash
# Extract test results
pio device monitor --quiet | grep "HARDWARE_TEST:" > test_results.log

# Check for failures
if grep -q "TESTS FAILED" test_results.log; then
    exit 1
fi
```

## Comparison with Mock Tests

| Aspect | Mock Tests | Hardware Tests |
|--------|------------|----------------|
| **Speed** | Very Fast (ms) | Moderate (seconds) |
| **Hardware Required** | None | ESP32-S3 board |
| **Coverage** | API Logic | Real Hardware + API |
| **USB Testing** | Simulated | Actual USB stack |
| **Memory Testing** | Simulated | Real heap behavior |
| **Timing Testing** | Mock delays | Real system timing |
| **CI/CD Complexity** | Simple | Hardware setup required |
| **Development Cycle** | Instant feedback | Flash and monitor |

## Future Enhancements

### Planned Features
1. **Automated P-touch Device Testing**: When P-touch printer connected
2. **Network Stress Testing**: WiFi performance and reliability
3. **Power Management Testing**: Sleep modes and power consumption
4. **Advanced USB Device Testing**: With various USB devices
5. **Multi-board Testing**: Parallel testing on multiple boards
6. **Test Result Database**: Historical performance tracking

### Integration Opportunities
1. **Hardware-in-the-Loop (HIL)**: Automated test execution
2. **Performance Regression Detection**: Benchmark tracking
3. **Real Device Validation**: Automatic P-touch printer testing
4. **Stress Testing**: Long-duration reliability tests

## Contributing

When adding new hardware tests:

1. **Follow Naming Conventions**: Use descriptive test names
2. **Add Comprehensive Logging**: Include ESP_LOGI statements
3. **Test Error Cases**: Include failure scenarios
4. **Document Requirements**: Note any special hardware needs
5. **Update This README**: Document new test categories

## Support

For hardware testing issues:

1. **Check Hardware Compatibility**: Ensure board supports features being tested
2. **Verify Power Supply**: USB Host tests need adequate power
3. **Update ESP-IDF**: Use latest stable version
4. **Check Connections**: Verify USB and power connections
5. **Review Logs**: Enable verbose output for debugging

This hardware testing framework provides comprehensive validation of ESP32-S3 functionality, serving as a crucial bridge between mock testing and full P-touch integration testing. 