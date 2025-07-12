# P-touch ESP32 Label Printer Server

A web-based label printing server for Brother P-touch printers using ESP32-S3 with USB Host support.

## Features

- 🖨️ **Brother P-touch Printer Support**: Compatible with 20+ Brother P-touch models
- 🌐 **Web Interface**: Modern, responsive web interface for label design and printing
- 📱 **Mobile Friendly**: Works on desktop, tablet, and mobile devices
- 🔌 **USB Host**: Direct USB connection to printers (no drivers needed)
- 🎨 **Label Design**: Text labels and simple graphics design tools
- 📊 **Real-time Status**: Live printer status monitoring via WebSocket
- 🏷️ **Multiple Formats**: Support for various tape sizes and types
- 🔄 **Auto-reconnect**: Automatic printer detection and reconnection

## Supported Printers

- PT-D450, PT-D460BT, PT-D410, PT-D600, PT-D610BT
- PT-P700, PT-P750W, PT-P710BT
- PT-H500, PT-E500, PT-E310BT
- PT-2300, PT-2420PC, PT-2450PC, PT-2430PC, PT-2700, PT-2730
- PT-1230PC, PT-1950
- PT-9200DX
- And many more...

## Hardware Requirements

- **ESP32-S3-DevKitC-1** (or compatible ESP32-S3 board with USB Host support)
- **Brother P-touch Label Printer** (see supported models above)
- **USB Cable** (USB-A to Micro-B or USB-C depending on printer model)
- **WiFi Network**

## Installation

### 1. Download and Setup

```bash
# Clone the repository
git clone https://github.com/tanvach/ptouch-esp32.git
cd ptouch-esp32

# Install PlatformIO if not already installed
pip install platformio
```

### 2. Quick Configuration

**Option A: Automated Setup (Recommended)**
```bash
# Run the setup script (creates config.h and provides guidance)
./setup.sh
```

**Option B: Manual Setup**
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

**Note**: The `config.h` file is automatically excluded from version control, so your credentials remain secure when publishing to GitHub.

### 3. Hardware Setup

1. Connect your Brother P-touch printer to the ESP32-S3 via USB
2. Power on both the ESP32-S3 and the printer
3. The ESP32 will automatically detect the printer on boot

### 4. Build and Upload

```bash
# Build the project
pio run

# Upload firmware to ESP32-S3
pio run --target upload

# Upload web interface files to ESP32-S3
pio run --target uploadfs

# Monitor serial output to see the assigned IP address
pio device monitor
```

## Usage

### Web Interface

1. After uploading, open the Serial Monitor to see the IP address
2. Navigate to `http://[ESP32_IP_ADDRESS]` in your web browser
3. The interface will show:
   - **Printer Status**: Connection status, tape info, and printer model
   - **Text Labels**: Simple text input with font size options
   - **Design Tools**: Canvas-based design tools for custom labels
   - **Print Queue**: Queue management for multiple print jobs

### API Endpoints

The server provides REST API endpoints for integration:

- `GET /api/status` - Get printer status
- `POST /api/print/text` - Print text label
- `POST /api/print/image` - Print image (base64 encoded)
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

The `PtouchPrinter` class can be used in your own Arduino projects:

```cpp
#include "ptouch_esp32.h"

PtouchPrinter printer;

void setup() {
    Serial.begin(115200);
    
    // Initialize printer
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
    // Check printer status
    if (printer.isConnected()) {
        if (printer.getStatus()) {
            Serial.println("Printer is ready");
        }
    }
    
    delay(1000);
}
```

## Project Structure

```
ptouch-esp32/
├── platformio.ini          # PlatformIO configuration
├── setup.sh               # Automated setup script
├── include/
│   └── config.example.h   # Configuration template
├── src/
│   └── main.cpp           # Main application
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
├── examples/              # Usage examples
│   └── basic_printing/
│       └── basic_printing.ino
└── README.md
```

## Development

### Building

```bash
# 1. Configure your WiFi credentials first
./setup.sh

# 2. Build the project
pio run

# 3. Upload firmware to ESP32-S3
pio run --target upload

# 4. Upload web interface files to ESP32-S3
pio run --target uploadfs

# 5. Monitor serial output to see IP address
pio device monitor
```

### Adding New Printer Models

To add support for new Brother P-touch models:

1. Add the printer information to `supported_devices[]` in `ptouch_printer.cpp`
2. Include the USB VID/PID and printer-specific flags
3. Test with the new printer model

### Web Interface Development

The web interface files are in the `data/` folder:
- `index.html` - Main HTML structure
- `style.css` - Styling and responsive design
- `script.js` - JavaScript functionality and WebSocket communication

## Troubleshooting

### Configuration Issues

1. **Missing config.h file**: Run `./setup.sh` or manually copy `include/config.example.h` to `include/config.h`
2. **WiFi connection fails**: Double-check SSID and password in `include/config.h`
3. **Compilation errors**: Ensure `config.h` exists and has valid syntax
4. **Setup script not executable**: Run `chmod +x setup.sh` first

### Printer Not Detected

1. Ensure the printer is powered on and connected via USB
2. Check that the printer is not in P-Lite mode (switch to position "E")
3. Verify the USB cable is working
4. Check the Serial Monitor for error messages

### Connection Issues

1. Verify WiFi credentials are correct in `include/config.h`
2. Check that the ESP32 and your device are on the same network
3. Try accessing the IP address directly (shown in serial monitor)
4. Check firewall settings
5. Ensure ESP32 is connected to WiFi (check serial output)

### Print Quality Issues

1. Ensure the tape is properly installed
2. Check tape width matches the printer specifications
3. Verify the label design fits within the printable area
4. Clean the print head if necessary

## Security

### Configuration Management

This project uses a secure configuration system to protect sensitive information:

- **Template File**: `include/config.example.h` - Version controlled template
- **Local Config**: `include/config.h` - Your actual credentials (Git ignored)
- **Auto-Setup**: Run `./setup.sh` to automatically create your config file

### Safe for GitHub

The configuration system ensures:
- ✅ WiFi credentials are never committed to version control
- ✅ `config.h` is automatically excluded via `.gitignore`
- ✅ Template file shows required configuration format
- ✅ Setup script simplifies initial configuration

### Setup Process

```bash
# Quick setup (creates config.h from template)
./setup.sh

# Or manually:
cp include/config.example.h include/config.h
nano include/config.h  # Edit with your credentials
```

### Publishing to GitHub

When publishing this project:
1. Your `config.h` file with credentials will be automatically excluded
2. The `config.example.h` template will be included for other users
3. Users can run `./setup.sh` to quickly configure their own credentials

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- Original [ptouch-print](https://git.familie-radermacher.ch/linux/ptouch-print.git) library by Dominic Radermacher
- ESP32 Arduino Core by Espressif Systems
- WebSocket and HTTP libraries by the Arduino community

## Support

For issues and questions:
- Open an issue on GitHub
- Check the troubleshooting section above
- Review the Brother P-touch printer documentation

---

**Note**: This project is not affiliated with Brother Industries, Ltd. Brother and P-touch are trademarks of Brother Industries, Ltd. 