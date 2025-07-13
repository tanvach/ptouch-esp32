# P-touch ESP32 Label Printer Server

A web-based label printing server for Brother P-touch printers using **ESP32-S3-DevKitC-1** with ESP-IDF framework and USB Host support.

**Based on the [ptouch-print](https://git.familie-radermacher.ch/linux/ptouch-print.git) library** - A comprehensive Brother P-touch protocol implementation with extensive debugging and fixes.

## 🚨 **IMPORTANT DISCLAIMER**

**⚠️ EXPERIMENTAL SOFTWARE - NOT TESTED WITH ACTUAL HARDWARE ⚠️**

This project is in **early development** and has **NOT been tested with any real Brother P-touch printers**. Before using this code:

- **❌ Do NOT use in production environments**
- **❌ Do NOT expect it to work with real printers without testing**
- **❌ No guarantee of compatibility with any Brother P-touch model**
- **❌ No validation of print quality or reliability**
- **❌ USB Host functionality is theoretical and untested**

**🧪 This is a development project that needs hardware testing and validation before any practical use.**

## 📜 **License**

This project is licensed under the **GNU General Public License v3.0** (GPL-3.0) as it is a derivative work of the [ptouch-print](https://git.familie-radermacher.ch/linux/ptouch-print.git) library.

- **License**: [GPL-3.0](LICENSE)
- **Original Work**: [ptouch-print library](https://git.familie-radermacher.ch/linux/ptouch-print.git) by Familie Radermacher
- **Derivative Work**: ESP32-S3 port with web interface and USB Host implementation

This means you are free to use, modify, and distribute this software under the terms of the GPL-3.0 license. See the [LICENSE](LICENSE) file for the full license text.

## 🎯 **Target Hardware**

This project is specifically designed for the **[ESP32-S3-DevKitC-1 Development Board](https://www.espboards.dev/esp32/esp32-s3-devkitc-1/)** with the following specifications:

- **Microcontroller**: ESP32-S3 (Xtensa dual-core, 240MHz)
- **Flash Memory**: **8MB** (standard variant)
- **RAM**: 320KB
- **Connectivity**: WiFi 802.11 b/g/n (2.4GHz), Bluetooth 5.0 + BLE 5.0
- **USB**: Native USB Host support with ESP32-S3
- **Architecture**: Xtensa
- **GPIO Pins**: 22 digital I/O pins, 16 external interrupt pins, 6 analog input pins

*Board specifications referenced from [ESPBoards.dev](https://www.espboards.dev/esp32/esp32-s3-devkitc-1/)*

## 🚧 **Current Status - DEVELOPMENT/TESTING PHASE**

⚠️ **WARNING: This project is in active development and has NOT been tested with actual hardware. Do not use in production environments.**

### **✅ Software Components Implemented**
- **ESP-IDF Framework**: Ported to ESP-IDF v5.4.1 with configuration
- **USB Host Stack**: USB Host API implementation (untested with real printers)
- **WiFi Connectivity**: 802.11 b/g/n connection with auto-reconnect
- **Web Server**: HTTP server with embedded interface
- **REST API**: API endpoints for printer control (backend only)
- **Brother P-touch Protocol**: Protocol implementation ported from ptouch-print library
- **Memory Optimization**: Builds successfully with reasonable resource usage
- **Debug System**: USB packet logging and protocol analysis framework

### **❌ CRITICAL GAPS - HARDWARE TESTING REQUIRED**
- **❌ No Real Printer Testing**: Zero validation with actual Brother P-touch printers
- **❌ USB Communication**: USB Host implementation unverified with real hardware
- **❌ Print Quality**: No testing of actual print output or quality
- **❌ Protocol Compatibility**: Brother P-touch protocol untested on ESP32-S3
- **❌ Error Handling**: Real-world error scenarios not validated
- **❌ Performance**: No measurement of actual print speeds or reliability

### **📊 Build Statistics**
- **Build Status**: ✅ **SUCCESS** (compiles without errors)
- **Compilation Time**: ~44 seconds
- **Flash Usage**: 29.0% (911,804 / 3,145,728 bytes)
- **RAM Usage**: 10.6% (34,896 / 327,680 bytes)
- **Available Space**: **2.2MB flash remaining** for future features

**Important Notice:** The Brother P-touch protocol implementation is derived from the extensively tested [ptouch-print](https://git.familie-radermacher.ch/linux/ptouch-print.git) library. However, this ESP32-S3 port has not been validated with actual hardware and should be considered experimental until comprehensive testing is completed.

## ✨ Intended Features

- 🖨️ **Brother P-touch Printer Support**: Protocol implementation for 25+ Brother P-touch models (from ptouch-print)
- 🌐 **Modern Web Interface**: Responsive, mobile-friendly design with embedded assets
- 📱 **Cross-Platform**: Should work on desktop, tablet, and mobile devices
- 🔌 **Native USB Host**: Direct USB connection using ESP32-S3 USB Host APIs
- 🎨 **Label Design Tools**: Text labels with formatting and layout options
- 📊 **Real-time Monitoring**: Live printer status via WebSocket connection
- 🏷️ **Multiple Tape Formats**: Support for various tape sizes and types
- 🔄 **Auto-Recovery**: Automatic USB reconnection and error recovery
- ⚡ **High Performance**: 240MHz dual-core processing with PSRAM support
- 🛡️ **Error Handling**: Comprehensive error handling and stability (when tested)

## 🖨️ Potentially Supported Printers

**⚠️ WARNING: Printer compatibility claims are theoretical and based on protocol implementation only. No actual testing has been performed.**

### **D-Series (Professional)**
- PT-D450, PT-D460BT, PT-D410, PT-D600, PT-D610BT

### **P-Series (Handheld)**
- PT-P700, PT-P750W, PT-P710BT, PT-P300BT

### **H-Series (Industrial)**
- PT-H500, PT-H300, PT-H300LI, PT-E500, PT-E310BT

### **Classic PC-Connected Series**
- PT-2300, PT-2420PC, PT-2450PC, PT-2430PC, PT-2700, PT-2730
- PT-1230PC, PT-1950, PT-1960
- PT-9200DX, PT-9700PC, PT-9800PCN

### **And Many More...**
*Protocol support derived from ptouch-print library (25+ models) - actual compatibility unverified*

## 🔧 Hardware Requirements

### **Primary Target Board**
- **[ESP32-S3-DevKitC-1](https://www.espboards.dev/esp32/esp32-s3-devkitc-1/)** with **8MB Flash** (standard variant)
  - Available from: [Amazon](https://www.amazon.com/s?k=ESP32-S3-DevKitC-1), [AliExpress](https://www.aliexpress.com/w/wholesale-esp32-s3-devkitc-1.html), electronics distributors
  - Price range: ~$15-25 USD

### **Additional Requirements**
- **Brother P-touch Label Printer** (from supported list)
- **USB-A to USB-B Cable** (standard printer cable)
- **WiFi Network** (2.4GHz, 802.11 b/g/n)
- **Power Supply**: 5V via USB-C (ESP32-S3-DevKitC-1)

### **Optional Enhancements**
- **External Power Supply**: For high-power printers
- **USB Hub**: For multiple printer support
- **Case/Enclosure**: For permanent installations

## 🚀 Installation

### **1. Install Development Environment**

**Option A: Using uv (Recommended)**
```bash
# Install uv package manager
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env

# Install PlatformIO
uv tool install platformio
```

**Option B: Using pip**
```bash
# Install PlatformIO
pip install platformio
```

### **2. Clone and Setup Project**

```bash
# Clone the repository
git clone https://github.com/tanvach/ptouch-esp32.git
cd ptouch-esp32

# Copy configuration template
cp include/config.example.h include/config.h
```

### **3. Configure WiFi and Debug**

**Configuration**: Edit the settings in `include/config.h`:

```cpp
// Configuration settings in include/config.h
// WiFi Configuration
#define WIFI_SSID "Your_WiFi_Network"      // 🔧 Set your WiFi network name
#define WIFI_PASSWORD "Your_WiFi_Password"  // 🔧 Set your WiFi password

// Web server configuration
#define WEB_SERVER_PORT 80

// USB Debug configuration
#define ENABLE_USB_DEBUG true   // 🔧 Set to true for debugging!
#define USB_DEBUG_LEVEL PTOUCH_DEBUG_LEVEL_INFO  // Set debug level
```

**Debug Levels Available**:
- `PTOUCH_DEBUG_LEVEL_NONE` - No debug output (production)
- `PTOUCH_DEBUG_LEVEL_ERROR` - Errors only
- `PTOUCH_DEBUG_LEVEL_WARN` - Warnings + errors
- `PTOUCH_DEBUG_LEVEL_INFO` - General info (recommended for debugging)
- `PTOUCH_DEBUG_LEVEL_DEBUG` - Detailed logging with hex dumps
- `PTOUCH_DEBUG_LEVEL_VERBOSE` - Everything (very detailed)

**Debug Features**: When enabled, you get comprehensive USB packet logging, Brother P-touch protocol analysis, interactive console commands, and performance statistics - perfect for troubleshooting printer communication!

### **4. Build and Deploy**

```bash
# Build for ESP32-S3-DevKitC-1
pio run --environment esp32-s3-devkitc-1

# Upload firmware
pio run --environment esp32-s3-devkitc-1 --target upload

# Monitor serial output
pio device monitor --baud 115200
```

**Expected Output:**
```
I (2340) main: WiFi connected to: Your_WiFi_Network
I (2345) main: IP address: 192.168.1.100
I (2350) main: Web server started on port 80
I (2355) usb_host: USB Host initialized successfully
I (2360) main: System ready - connect your printer!
```

## 📱 Theoretical Usage (UNTESTED)

**⚠️ WARNING: The following usage instructions are theoretical and based on intended functionality. None of this has been tested with actual hardware.**

### **Intended Web Interface Workflow**

1. **Access Interface**: Navigate to `http://[ESP32_IP_ADDRESS]` in your browser (if WiFi works)
2. **Connect Printer**: Plug your Brother P-touch printer into the ESP32-S3 via USB (may not work)
3. **Verify Connection**: Check status panel for printer detection (unverified)
4. **Start Printing**: Use the web interface to create and print labels (untested)

### **Intended Features (NOT VALIDATED):**
- **📊 Status Dashboard**: Real-time printer status and connection info (theoretical)
- **✏️ Text Labels**: Rich text input with formatting options (web UI only)
- **📐 Layout Tools**: Margin, alignment, and sizing controls (not tested)
- **🎨 Design Options**: Font selection and text styling (implementation unclear)
- **📋 Print Queue**: Job management and history (not implemented)
- **⚙️ Settings**: Printer configuration and preferences (not implemented)

### **Intended API Endpoints (UNTESTED)**

**⚠️ WARNING: These API endpoints are theoretical and have not been tested with actual hardware.**

```bash
# Get printer status (may return placeholder data)
curl http://[ESP32_IP]/api/status

# Print text label (may fail with real printers)
curl -X POST http://[ESP32_IP]/api/print/text \
  -H "Content-Type: application/json" \
  -d '{"text": "Hello World!", "margin": 3}'

# Reconnect printer (functionality unverified)
curl -X POST http://[ESP32_IP]/api/reconnect

# List supported printers (theoretical list only)
curl http://[ESP32_IP]/api/printers
```

### **Intended WebSocket Communication (UNTESTED)**

**⚠️ WARNING: WebSocket functionality is theoretical and has not been tested.**

```javascript
// Connect to real-time updates (may not work)
const ws = new WebSocket('ws://[ESP32_IP]/ws');

// Listen for status updates (data may be placeholder)
ws.onmessage = (event) => {
    const status = JSON.parse(event.data);
    console.log('Printer:', status.connected ? 'Connected' : 'Disconnected');
    console.log('Model:', status.model);
    console.log('Status:', status.printerStatus);
};

// Send print commands (may fail with real printers)
ws.send(JSON.stringify({
    command: 'printText',
    text: 'Hello World!',
    options: { margin: 3, copies: 1 }
}));
```

## 🔬 Technical Implementation

### **ESP-IDF Framework (v5.4.1)**
- **Native USB Host APIs**: Direct hardware access with `usb_host.h`
- **Advanced WiFi Management**: Dual-mode support with `esp_wifi.h`
- **HTTP Server**: High-performance `esp_http_server.h` implementation
- **FreeRTOS**: Dual-core task management and real-time scheduling
- **SPIFFS**: Embedded file system for web assets
- **PSRAM Support**: Extended memory management

### **USB Host Architecture**
```
ESP32-S3 USB Host Stack
├── Device Enumeration & Detection
├── Endpoint Configuration & Management  
├── Bulk Transfer Implementation
├── Error Handling & Recovery
├── Status Monitoring & Events
└── Brother P-touch Protocol Layer
```

### **Performance Optimizations**
- **Dual-Core Processing**: Protocol handling on dedicated core
- **PSRAM Integration**: Extended memory for large print jobs
- **DMA Transfers**: Hardware-accelerated USB communication
- **Interrupt-Driven I/O**: Non-blocking operations
- **Connection Pooling**: Persistent WebSocket connections

### **Protocol Implementation**

**Recent Major Bug Fixes:**
- ✅ **Initialization Sequence**: Fixed 102-byte invalidate command
- ✅ **PackBits Compression**: Corrected compression command to `{0x4d, 0x02}`
- ✅ **Info Command**: Fixed byte positioning and media width settings
- ✅ **D460BT Magic Commands**: Updated magic sequences for newer models
- ✅ **Raster Data**: Implemented proper PackBits compression
- ✅ **Print Finalization**: Corrected eject (0x1a) and chain (0x0c) commands
- ✅ **Status Parsing**: Enhanced error detection and reporting

## 📁 Project Structure

```
ptouch-esp32/
├── platformio.ini              # ESP32-S3 configuration
├── sdkconfig.defaults          # ESP-IDF optimized settings  
├── default_8MB.csv            # Custom partition table (8MB)
├── include/
│   ├── config.example.h       # Configuration template
│   └── ptouch_esp32.h         # Main library header
├── src/
│   └── main.cpp               # Application entry point (ESP-IDF)
├── components/
│   └── ptouch-esp32/          # ESP-IDF component
│       ├── CMakeLists.txt     # Component build config
│       ├── include/
│       │   └── ptouch_esp32.h # Public API
│       └── src/
│           ├── ptouch_printer.cpp    # Core printer logic
│           ├── ptouch_printing.cpp   # Print job management
│           ├── ptouch_image.cpp      # Image processing
│           └── ptouch_utils.cpp      # Utility functions
└── examples/
    └── basic_printing/        # Arduino-style examples
        └── basic_printing.ino
```

## 🧪 Testing Status

### **✅ Limited Software Testing**
- **ESP-IDF Build System**: Compilation successful
- **USB Host Stack**: Code compiles but hardware enumeration NOT tested
- **WiFi Connectivity**: Basic connection code implemented (not stress tested)
- **Web Server**: Basic HTTP server code (not load tested)
- **Protocol Implementation**: Code review against original library only
- **Memory Management**: No stress testing performed
- **Error Handling**: Code paths exist but not validated with real scenarios

### **🔬 CRITICAL HARDWARE TESTING NEEDED**
- **❌ NO ACTUAL PRINTER TESTING**: Zero validation with any Brother P-touch printer
- **❌ USB Host Functionality**: ESP32-S3 USB Host communication not validated
- **❌ Print Quality Assessment**: No print output testing whatsoever
- **❌ Protocol Compatibility**: Brother P-touch protocol not tested on ESP32-S3
- **❌ Long-term Stability**: No extended operation testing
- **❌ Edge Case Handling**: No testing of unusual printer behaviors
- **❌ Performance Benchmarking**: No measurement of actual print speeds
- **❌ Error Recovery**: Real-world error scenarios not tested

### **🧪 Automated Testing Framework**

A comprehensive **mock-based testing suite** is available for software validation:

📖 **[Complete Testing Guide](test/TESTING_GUIDE.md)** - Detailed documentation covering:
- **Test Architecture**: Custom lightweight framework with hardware mocks
- **Build & Run Instructions**: Step-by-step setup and execution
- **Test Categories**: 22 tests covering USB Host, protocol, integration scenarios
- **Mock Capabilities**: Simulated Brother P-touch printers with error injection
- **Blind Spots**: Honest assessment of what's NOT tested (real hardware, timing, electrical)
- **Future Roadmap**: Plans for hardware-in-the-loop testing

**Quick Start:**
```bash
cd test
mkdir -p build && cd build
cmake .. && make
./ptouch_tests --verbose
```

**✅ All 22 tests passing** with comprehensive USB Host simulation, protocol validation, and error handling scenarios.

## 🤝 Contributing

**This project URGENTLY needs hardware testing before any other development work.**

### **🚨 CRITICAL: Hardware Testing Required**

**This is the #1 priority.** Before any other features or improvements:

- **Test with actual Brother P-touch printers** (any model)
- **Report whether USB Host detection works at all**
- **Document any printer communication (success or failure)**
- **Share detailed error messages and logs**
- **Test basic print functionality**

### **📋 Essential Testing Report Format**

If you test with hardware, please provide:
- **ESP32-S3 Board**: Exact model and specifications
- **Printer Model**: Exact model number and firmware version
- **Connection Results**: Success/failure with detailed error messages
- **USB Host Behavior**: Device enumeration, communication attempts
- **Print Results**: Did anything print? Quality? Errors?
- **Serial Output**: Complete logs with debug enabled
- **Issues Found**: Every bug, crash, or unexpected behavior

### **📈 Secondary Development Areas (AFTER hardware testing)**
- Fix hardware compatibility issues
- Additional printer model support
- Advanced label design features
- Performance optimizations
- Mobile app development
- Cloud integration

## 🔧 Troubleshooting (THEORETICAL)

**⚠️ WARNING: Since this project hasn't been tested with actual hardware, these troubleshooting steps are theoretical and may not work.**

### **USB Host Issues (UNTESTED)**
```bash
# Check USB Host initialization
pio device monitor --filter=esp32_exception_decoder
```

**Potential Solutions (NOT VALIDATED):**
- **Printer Not Detected**: Try different USB cable, check power (may not help)
- **Connection Timeouts**: Increase timeout in configuration (untested)
- **Transfer Errors**: Verify adequate power supply (5V, 2A recommended) (theoretical)
- **Enumeration Failures**: Some printers need warm-up time (assumption only)

### **Build Issues**
```bash
# Clean build environment
pio run --target clean

# Verify ESP-IDF configuration  
pio run --environment esp32-s3-devkitc-1 --verbose
```

**Configuration Checks:**
- **Flash Size**: Verify 8MB flash detection
- **Partition Table**: Ensure `default_8MB.csv` is used
- **Board Type**: Confirm `esp32-s3-devkitc-1` target
- **Dependencies**: Check ArduinoJson library installation

### **Network Issues**
- **WiFi Connection**: Verify 2.4GHz network (5GHz not supported)
- **IP Assignment**: Check DHCP configuration
- **Firewall**: Ensure port 80 access allowed
- **Browser Cache**: Clear cache if web interface doesn't load

## 📊 Memory Usage

**Current Optimization (8MB Flash):**
- **Application Size**: 897KB (28.5% of flash)
- **Available Flash**: 2.2MB for future features
- **RAM Usage**: 34KB (10.6% of 320KB)
- **Partition Layout**:
  - Application: 3MB
  - Storage (FAT): 1MB  
  - SPIFFS: 4MB
  - System: 256KB

## 📄 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for full details.

## 🙏 Acknowledgments

- **[ptouch-print](https://git.familie-radermacher.ch/linux/ptouch-print.git)** by Dominic Radermacher - Original protocol implementation
- **[Espressif Systems](https://www.espressif.com/)** - ESP-IDF framework and ESP32-S3 hardware
- **[ESP Boards Database](https://www.espboards.dev/)** - Comprehensive ESP32 board specifications
- **[uv Package Manager](https://github.com/astral-sh/uv)** by Astral - Modern Python package management

## 🆘 Support

**Getting Help:**
- 🐛 **Bug Reports**: [Open GitHub Issue](https://github.com/tanvach/ptouch-esp32/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/tanvach/ptouch-esp32/discussions)  
- 📖 **Documentation**: Check troubleshooting section above
- 🤝 **Community**: Hardware testing reports welcome!

**Before Opening Issues:**
1. Check existing issues and discussions
2. Include printer model and ESP32-S3 board details
3. Provide serial monitor output
4. Include steps to reproduce problems

---

## 📜 **License & Attribution**

This project is licensed under the **GNU General Public License v3.0** (GPL-3.0).

### **Original Work Attribution**
- **Based on**: [ptouch-print library](https://git.familie-radermacher.ch/linux/ptouch-print.git)
- **Original Authors**: Familie Radermacher and contributors
- **Original License**: GPL-3.0
- **Derivative Work**: ESP32-S3 port with web interface and USB Host implementation

### **GPL-3.0 License Summary**
- ✅ **Commercial Use**: Permitted
- ✅ **Modification**: Permitted  
- ✅ **Distribution**: Permitted
- ✅ **Private Use**: Permitted
- ❗ **License and Copyright Notice**: Required
- ❗ **State Changes**: Must document modifications
- ❗ **Disclose Source**: Must provide source code
- ❗ **Same License**: Derivative works must use GPL-3.0

For the full license text, see the [LICENSE](LICENSE) file.

### **Contributing**
By contributing to this project, you agree that your contributions will be licensed under the GPL-3.0 license.

---

**Disclaimer**: This project is not affiliated with Brother Industries, Ltd. Brother and P-touch are registered trademarks of Brother Industries, Ltd. This software is provided "as is" without warranty of any kind as specified in the GPL-3.0 license.

**⚠️ FINAL WARNING: This is experimental software that has NOT been tested with actual Brother P-touch printers. The author makes no claims about functionality, compatibility, or reliability. Use at your own risk and do not expect it to work without extensive testing and debugging. This is a development project requiring hardware validation before any practical use.** 