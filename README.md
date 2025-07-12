# P-touch ESP32 Label Printer Server

A web-based label printing server for Brother P-touch printers using ESP32-S3.

## ⚠️ **Current Status**

This project is **under development**. The web server and interface are functional, but **USB Host functionality is not currently available** in the Arduino ESP32 framework.

**What Works:**
- ✅ Web server with responsive interface
- ✅ WiFi connectivity and configuration
- ✅ REST API endpoints
- ✅ WebSocket communication
- ✅ Printer status simulation
- ✅ Project compiles and runs successfully

**What Doesn't Work Yet:**
- ❌ Direct USB connection to printers (Arduino framework limitation)
- ❌ Actual printing functionality
- ❌ Real printer detection and status

**Roadmap:**
- 🔄 Convert to ESP-IDF framework for full USB Host support
- 🔄 Add USB Host Shield support as alternative
- 🔄 Explore WiFi/Bluetooth printer connectivity

## Features (Planned/Partial)

- 🖨️ **Brother P-touch Printer Support**: Compatible with 20+ Brother P-touch models (when USB Host is implemented)
- 🌐 **Web Interface**: Modern, responsive web interface for label design and printing
- 📱 **Mobile Friendly**: Works on desktop, tablet, and mobile devices
- 🔌 **USB Host**: Direct USB connection to printers *(requires ESP-IDF framework)*
- 🎨 **Label Design**: Text labels and simple graphics design tools *(web interface ready)*
- 📊 **Real-time Status**: Live printer status monitoring via WebSocket *(simulated)*
- 🏷️ **Multiple Formats**: Support for various tape sizes and types *(when printing works)*
- 🔄 **Auto-reconnect**: Automatic printer detection and reconnection *(when USB Host works)*

## Supported Printers (When USB Host is Implemented)

- PT-D450, PT-D460BT, PT-D410, PT-D600, PT-D610BT
- PT-P700, PT-P750W, PT-P710BT
- PT-H500, PT-E500, PT-E310BT
- PT-2300, PT-2420PC, PT-2450PC, PT-2430PC, PT-2700, PT-2730
- PT-1230PC, PT-1950
- PT-9200DX
- And many more...

## Hardware Requirements

- **ESP32-S3-DevKitC-1** (or compatible ESP32-S3 board)
- **Brother P-touch Label Printer** (for future USB Host implementation)
- **USB Cable** (for future use)
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

**Alternative Installation Methods:**
```bash
# Using pip (if you prefer)
pip install platformio

# Using conda
conda install -c conda-forge platformio

# Using brew (macOS)
brew install platformio
```

### 2. Download and Setup

```bash
# Clone the repository
git clone https://github.com/tanvach/ptouch-esp32.git
cd ptouch-esp32
```

### 3. Configuration

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

### 4. Build and Upload

```bash
# Build the project
pio run

# Upload firmware to ESP32-S3
pio run --target upload

# Monitor serial output to see the assigned IP address
pio device monitor
```

## Current Usage

### Web Interface

1. After uploading, open the Serial Monitor to see the IP address
2. Navigate to `http://[ESP32_IP_ADDRESS]` in your web browser
3. The interface will show:
   - **Printer Status**: Simulated connection status and printer info
   - **Text Labels**: Text input interface (printing not functional yet)
   - **Print Queue**: Interface for future print job management

### API Endpoints

The server provides REST API endpoints (printing commands return errors about USB Host limitations):

- `GET /api/status` - Get simulated printer status
- `POST /api/print/text` - Text printing interface (returns error)
- `POST /api/print/image` - Image printing interface (not implemented)
- `POST /api/reconnect` - Reconnection attempt (returns error)
- `GET /api/printers` - List supported printers

### WebSocket Events

Real-time communication via WebSocket at `/ws`:

```javascript
// Connect to WebSocket
const ws = new WebSocket('ws://[ESP32_IP]/ws');

// Send commands (will receive error responses)
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

## Library Usage (Future)

When USB Host functionality is implemented, the `PtouchPrinter` class will work like this:

```cpp
#include "ptouch_esp32.h"

PtouchPrinter printer;

void setup() {
    Serial.begin(115200);
    
    // Currently prints error messages about USB Host limitations
    if (printer.begin()) {
        if (printer.detectPrinter()) {
            if (printer.connect()) {
                Serial.println("Printer connected!");
                
                // These will work when USB Host is implemented
                printer.printText("Hello World!");
                
                uint8_t bitmap[16] = {0xFF, 0x81, 0x81, 0xFF, ...};
                printer.printBitmap(bitmap, 32, 8);
            }
        }
    }
}

void loop() {
    // Currently returns false due to USB Host limitations
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
│   ├── config.example.h   # Configuration template
│   └── config.h           # Your credentials (Git ignored)
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

# 4. Monitor serial output to see IP address
pio device monitor
```

### Working on USB Host Support

To implement actual USB Host functionality, the project needs to be converted to use the ESP-IDF framework instead of Arduino:

1. **ESP-IDF Conversion**: Migrate to ESP-IDF which has full USB Host APIs
2. **USB Host Shield**: Add external USB Host controller hardware
3. **Alternative Connectivity**: Implement WiFi or Bluetooth printer support

### Adding New Printer Models

To add support for new Brother P-touch models (for future USB Host implementation):

1. Add the printer information to `supported_devices[]` in `ptouch_printer.cpp`
2. Include the USB VID/PID and printer-specific flags
3. Test with the new printer model when USB Host is working

### Web Interface Development

The web interface files are in the `data/` folder and are fully functional:
- `index.html` - Main HTML structure
- `style.css` - Styling and responsive design
- `script.js` - JavaScript functionality and WebSocket communication

## Troubleshooting

### Configuration Issues

1. **Missing config.h file**: Run `./setup.sh` or manually copy `include/config.example.h` to `include/config.h`
2. **WiFi connection fails**: Double-check SSID and password in `include/config.h`
3. **Compilation errors**: Ensure `config.h` exists and has valid syntax
4. **Setup script not executable**: Run `chmod +x setup.sh` first

### USB Host Error Messages

If you see messages like "ERROR: USB Host functionality not available in Arduino framework", this is expected. The current Arduino ESP32 framework doesn't provide the necessary USB Host APIs.

**Solutions:**
- Wait for ESP-IDF conversion (planned)
- Contribute to ESP-IDF conversion effort
- Use USB Host Shield hardware
- Explore WiFi/Bluetooth printer connectivity

### Connection Issues

1. Verify WiFi credentials are correct in `include/config.h`
2. Check that the ESP32 and your device are on the same network
3. Try accessing the IP address directly (shown in serial monitor)
4. Check firewall settings
5. Ensure ESP32 is connected to WiFi (check serial output)

### PlatformIO Installation Issues

**With uv (recommended):**
```bash
# If uv command not found, restart shell or run:
source $HOME/.local/bin/env

# If installation fails, try:
uv tool install --force platformio
```

**Alternative solutions:**
```bash
# Use pip instead
pip install platformio

# Or use conda
conda install -c conda-forge platformio
```

## Technical Details

### USB Host Limitation

The Arduino ESP32 framework doesn't expose the USB Host APIs that are available in ESP-IDF. This project includes placeholder implementations that:

- Compile successfully
- Display informative error messages
- Provide the correct API structure for future implementation

### Current Implementation

- **Web Server**: Fully functional using ESPAsyncWebServer
- **WiFi**: Complete WiFi connectivity and management
- **APIs**: REST and WebSocket endpoints work (return appropriate errors for printer functions)
- **Configuration**: Secure credential management system
- **Library Structure**: Complete P-touch printer library structure (ready for USB Host)

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

## Contributing

We welcome contributions, especially for:

1. **ESP-IDF Conversion**: Help convert the project to ESP-IDF framework
2. **USB Host Shield Support**: Add support for external USB Host controllers
3. **Alternative Connectivity**: WiFi or Bluetooth printer support
4. **Web Interface**: Improvements to the user interface
5. **Documentation**: Better setup guides and troubleshooting

### Development Process

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
- [uv](https://github.com/astral-sh/uv) package manager by Astral

## Support

For issues and questions:
- Open an issue on GitHub
- Check the troubleshooting section above
- Review the Brother P-touch printer documentation
- See current limitations section for known issues

---

**Note**: This project is not affiliated with Brother Industries, Ltd. Brother and P-touch are trademarks of Brother Industries, Ltd. 