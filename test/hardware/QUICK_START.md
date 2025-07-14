# Quick Start: ESP32-S3 Hardware Testing

## 🚀 Get Running in 5 Minutes

### Prerequisites
- ESP32-S3 development board
- USB cable
- PlatformIO or ESP-IDF installed
- **⚠️ NO USB CLIENT DEVICES REQUIRED** - Tests run without external USB devices

### Step 1: Navigate to Hardware Tests
```bash
cd test/hardware
```

### Step 2: Connect Your ESP32-S3
- Connect ESP32-S3 to your computer via USB
- Note the serial port (e.g., `/dev/ttyUSB0` or `COM3`)
- **DO NOT connect any USB devices to the ESP32-S3** - Current tests validate USB Host API only

### Step 3: Build and Flash

**Option A: ESP-IDF (Recommended)**
```bash
# Set up ESP-IDF environment (if not already done)
source ~/esp/esp-idf/export.sh  # or wherever you installed ESP-IDF

# Build and flash
idf.py set-target esp32s3
idf.py build flash monitor
```

**Option B: PlatformIO (May have toolchain issues on macOS)**
```bash
# Clean build first
pio run --target clean

# All tests
pio run --target upload && pio device monitor

# If you encounter toolchain errors, use ESP-IDF instead
```

### Step 4: Watch the Results
You should see output like:
```
I (1234) HARDWARE_TEST: ESP32-S3 P-touch Hardware Test Suite Starting...
I (1235) HARDWARE_TEST: Free memory: 390000 bytes
...
I (5684) HARDWARE_TEST: ALL TESTS PASSED!
```

## ✅ What Gets Tested

### USB Host Tests (8 tests)
- USB Host library installation/uninstallation
- Client registration and management
- Port power control
- Transfer allocation and management
- Memory usage monitoring
- Error handling and recovery
- Performance benchmarks

### Core Systems Tests (9 tests)
- GPIO operations (LED blinking)
- Timing accuracy (delays, timers)
- Memory management (allocation/deallocation)
- Random number generation
- FreeRTOS task management
- CPU performance benchmarks
- Watchdog functionality
- Heap capabilities (DMA, SPIRAM)
- System information and chip ID

## 🔧 Different Test Configurations

```bash
# Test only USB Host functionality
pio run -e esp32s3_usb_host_tests --target upload

# Test only core systems (no USB)
pio run -e esp32s3_core_tests --target upload

# Performance testing (release build)
pio run -e esp32s3_performance_tests --target upload

# Verbose output for debugging
pio run -e esp32s3_all_tests_verbose --target upload
```

## 📊 Expected Results

**Successful Test Run:**
- ✅ 15+ tests passing
- ✅ LED flashing during tests
- ✅ Memory usage reports
- ✅ Performance benchmarks

**If Tests Fail:**
- Check power supply (USB Host needs sufficient power)
- Verify ESP32-S3 board type
- Check serial connection
- Try verbose mode: `-e esp32s3_all_tests_verbose`

## 🏃‍♂️ Quick Validation

**Minimum viable test:** Just run core systems tests to validate basic ESP32-S3 functionality:
```bash
pio run -e esp32s3_core_tests --target upload && pio device monitor
```

**USB Host validation:** Test USB Host stack without needing USB devices:
```bash
pio run -e esp32s3_usb_host_tests --target upload && pio device monitor
```

## 🎯 Next Steps

1. **All tests passing?** → Ready for P-touch integration!
2. **USB Host tests failing?** → Check board compatibility and power
3. **Want to add tests?** → See `README.md` for detailed guide
4. **Ready for P-touch?** → Connect P-touch printer and run integration tests

## 💡 Pro Tips

- **LED Indicators**: Watch the built-in LED for test status
- **Serial Monitor**: Keep monitor open to see real-time results
- **Memory Monitoring**: Tests show real ESP32-S3 memory usage
- **Performance Data**: Get actual timing benchmarks for your board

## 🚨 Troubleshooting

**"No such file or directory"**
```bash
# Make sure you're in the right directory
cd test/hardware
ls  # Should see platformio.ini
```

**"Device not found"**
```bash
# Check connected devices
pio device list

# Try different USB port or cable
```

**"Build failed"**
```bash
# Clean and rebuild
pio run --target clean
pio run --target upload
```

**Tests fail with memory errors**
- Your ESP32-S3 board might have less RAM than expected
- Try performance tests: `pio run -e esp32s3_performance_tests --target upload`

Ready to test your ESP32-S3? Just run the commands above and watch your hardware get validated! 🎉

## ⚠️ Test Limitations

**WHAT THESE TESTS VERIFY:**
- ✅ ESP-IDF USB Host API functions work correctly
- ✅ Basic ESP32-S3 hardware functionality works  
- ✅ Memory management works properly
- ✅ Error handling behaves correctly

**WHAT THESE TESTS DO NOT VERIFY:**
- ❌ Actual P-touch printer communication
- ❌ Real USB device enumeration with connected devices
- ❌ Data transfer operations
- ❌ Protocol-specific communication

**Use these tests to verify your ESP32-S3 hardware and ESP-IDF installation work correctly before moving to real device testing.** 