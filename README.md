# P-touch ESP32 Label Printer Server

A web-based label printing server for Brother P-touch printers using ESP32-S3 with ESP-IDF framework.

## 🎉 **Current Status**

This project has been **successfully converted to ESP-IDF framework** with full USB Host support implemented.

**What Works:**
- ✅ ESP-IDF framework with USB Host APIs
- ✅ Web server with responsive interface
- ✅ WiFi connectivity and configuration
- ✅ REST API endpoints
- ✅ WebSocket communication
- ✅ USB Host stack implementation
- ✅ USB device detection and enumeration
- ✅ Brother P-touch printer protocol implementation
- ✅ Printer status monitoring and communication

**What's Ready for Testing:**
- 🧪 Direct USB connection to Brother P-touch printers
- 🧪 Actual printing functionality
- 🧪 Real printer detection and status

**Important Note:** The printer support is based on the original [ptouch-print](https://git.familie-radermacher.ch/linux/ptouch-print.git) library but is **mostly untested** with actual hardware. While the protocol implementation is complete, individual printer models may require additional testing and fine-tuning.

## Features

- 🖨️ **Brother P-touch Printer Support**: Protocol implementation for 25+ Brother P-touch models (based on original library, mostly untested)
- 🌐 **Web Interface**: Modern, responsive web interface for label design and printing
- 📱 **Mobile Friendly**: Works on desktop, tablet, and mobile devices
- 🔌 **USB Host**: Direct USB connection to printers using ESP-IDF USB Host APIs
- 🎨 **Label Design**: Text labels and simple graphics design tools
- 📊 **Real-time Status**: Live printer status monitoring via WebSocket
- 🏷️ **Multiple Formats**: Support for various tape sizes and types
- 🔄 **Auto-reconnect**: Automatic printer detection and reconnection

## Supported Printers

**Based on original ptouch-print library - mostly untested with ESP32:**

**D-Series:**
- PT-D450, PT-D460BT, PT-D410, PT-D600, PT-D610BT

**P-Series:**
- PT-P700, PT-P750W, PT-P710BT

**H-Series:**
- PT-H500, PT-E500, PT-E310BT

**Classic Series:**
- PT-2300, PT-2420PC, PT-2450PC, PT-2430PC, PT-2700, PT-2730
- PT-1230PC, PT-1950
- PT-9200DX

**And many more...**

⚠️ **Testing Status**: While the protocol implementation supports these printers based on the original library, **most have not been tested** with the ESP32 implementation. Community testing and feedback are welcome!

## Hardware Requirements

- **ESP32-S3-DevKitC-1** (or compatible ESP32-S3 board)
- **Brother P-touch Label Printer** (from supported list)
- **USB Cable** (USB-A to printer's USB connector)
- **WiFi Network**

## Installation

### 1. Install PlatformIO using uv (Recommended)

**Install uv (if not already installed):**
```bash
# Install uv package manager
curl -LsSf https://astral.sh/uv/install.sh | sh
source $HOME/.local/bin/env
```

**Install PlatformIO:**
```bash
# Install PlatformIO as an isolated tool using uv
uv tool install platformio
```

### 2. Download and Setup

```bash
# Clone the repository
git clone https://github.com/tanvach/ptouch-esp32.git
cd ptouch-esp32
```

### 3. Configuration

```bash
# Copy the configuration template
cp include/config.example.h include/config.h

# Edit the configuration file with your WiFi credentials
nano include/config.h
```

Update `include/config.h` with your WiFi credentials:

```cpp
// WiFi Configuration
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";

// Other configuration options are available...
```

### 4. Build and Upload

```bash
# Build the project
pio run

# Upload firmware to ESP32-S3
pio run --target upload

# Monitor serial output to see the assigned IP address
pio device monitor
```

## Usage

### Web Interface

1. After uploading, open the Serial Monitor to see the IP address
2. Navigate to `http://[ESP32_IP_ADDRESS]` in your web browser
3. Connect your Brother P-touch printer via USB
4. The interface will show:
   - **Printer Status**: Real-time connection status and printer info
   - **Text Labels**: Text input interface for printing
   - **Print Queue**: Print job management

### API Endpoints

The server provides REST API endpoints:

- `GET /api/status` - Get current printer status
- `POST /api/print/text` - Print text labels
- `POST /api/print/image` - Print image (future feature)
- `POST /api/reconnect` - Reconnect to printer
- `GET /api/printers` - List supported printers

### WebSocket Events

Real-time communication via WebSocket at `/ws`:

```javascript
// Connect to WebSocket
const ws = new WebSocket('ws://[ESP32_IP]/ws');

// Send commands
ws.send(JSON.stringify({
    command: 'printText',
    text: 'Hello World!'
}));

// Receive status updates
ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    console.log('Printer status:', data);
};
```

## Library Usage

```cpp
#include "ptouch_esp32.h"

PtouchPrinter printer;

void setup() {
    Serial.begin(115200);
    
    if (printer.begin()) {
        if (printer.detectPrinter()) {
            if (printer.connect()) {
                Serial.println("Printer connected!");
                
                // Print text
                printer.printText("Hello World!");
                
                // Print bitmap
                uint8_t bitmap[16] = {0xFF, 0x81, 0x81, 0xFF, ...};
                printer.printBitmap(bitmap, 32, 8);
            }
        }
    }
}

void loop() {
    if (printer.isConnected()) {
        if (printer.getStatus()) {
            Serial.println("Printer is ready");
        }
    }
    
    delay(1000);
}
```

## Technical Implementation

### ESP-IDF Framework

The project uses ESP-IDF framework for:
- Native USB Host APIs (`usb_host.h`)
- Advanced WiFi management (`esp_wifi.h`)
- HTTP server (`esp_http_server.h`)
- FreeRTOS task management
- SPIFFS file system support

### USB Host Implementation

- **Device Detection**: Automatic USB device enumeration
- **Protocol Support**: Full Brother P-touch command protocol
- **Error Handling**: Comprehensive error detection and recovery
- **Status Monitoring**: Real-time printer status updates
- **Bulk Transfers**: Efficient USB bulk transfer implementation

### Recent Bug Fixes

The implementation has been thoroughly debugged against the original ptouch-print library:
- ✅ Fixed initialization sequence (102-byte invalidate command)
- ✅ Corrected PackBits compression command
- ✅ Fixed info command byte positioning
- ✅ Corrected D460BT magic commands
- ✅ Fixed raster data handling
- ✅ Updated print finalization commands

## Project Structure

```
ptouch-esp32/
├── platformio.ini          # PlatformIO configuration (ESP-IDF)
├── sdkconfig.defaults      # ESP-IDF configuration
├── CMakeLists.txt         # ESP-IDF build system
├── include/
│   ├── config.example.h   # Configuration template
│   └── config.h           # Your credentials (Git ignored)
├── src/
│   └── main.cpp           # Main application (ESP-IDF)
├── lib/
│   └── ptouch-esp32/      # P-touch library
│       ├── include/
│       │   └── ptouch_esp32.h
│       └── src/
│           ├── ptouch_printer.cpp
│           ├── ptouch_printing.cpp
│           ├── ptouch_image.cpp
│           └── ptouch_utils.cpp
├── data/                  # Web interface files
│   ├── index.html
│   ├── style.css
│   └── script.js
└── README.md
```

## Testing Status

**Tested Components:**
- ✅ ESP-IDF framework compilation
- ✅ USB Host stack initialization
- ✅ WiFi connectivity
- ✅ Web server functionality
- ✅ Protocol implementation (code review)

**Needs Testing:**
- 🧪 Actual printer hardware communication
- 🧪 Individual printer model compatibility
- 🧪 Print quality and formatting
- 🧪 Error handling with real hardware
- 🧪 Long-term stability

## Contributing

We especially welcome contributions for:

1. **Hardware Testing**: Testing with actual Brother P-touch printers
2. **Printer Model Support**: Adding support for additional models
3. **Bug Reports**: Issues with specific printer models
4. **Documentation**: Better setup guides and troubleshooting
5. **Web Interface**: Improvements to the user interface

### Testing Contributions

If you test with actual hardware, please report:
- Printer model and firmware version
- Connection success/failure
- Print quality results
- Any error messages or issues
- Suggestions for improvements

## Troubleshooting

### USB Host Issues

1. **Printer not detected**: Check USB cable and try different USB port
2. **Connection fails**: Verify printer is powered on and in ready state
3. **USB Host errors**: Check ESP32-S3 has sufficient power supply
4. **Transfer timeouts**: Some printers may need longer timeout values

### Configuration Issues

1. **Missing config.h file**: Copy `include/config.example.h` to `include/config.h`
2. **WiFi connection fails**: Double-check SSID and password
3. **Compilation errors**: Ensure ESP-IDF configuration is correct
4. **Flash size errors**: Verify board configuration in `platformio.ini`

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- Original [ptouch-print](https://git.familie-radermacher.ch/linux/ptouch-print.git) library by Dominic Radermacher
- ESP-IDF framework by Espressif Systems
- USB Host implementation based on ESP-IDF examples
- [uv](https://github.com/astral-sh/uv) package manager by Astral

## Support

For issues and questions:
- Open an issue on GitHub with printer model and test results
- Check the troubleshooting section above
- Review the Brother P-touch printer documentation
- Community testing reports are very welcome!

---

**Note**: This project is not affiliated with Brother Industries, Ltd. Brother and P-touch are trademarks of Brother Industries, Ltd. 