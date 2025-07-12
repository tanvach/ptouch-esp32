#include "ptouch_esp32.h"
#include <Arduino.h>

// Supported printer models (ported from original library)
static const pt_dev_info supported_devices[] = {
    {0x04f9, 0x2001, "PT-9200DX", 384, 360, FLAG_RASTER_PACKBITS|FLAG_HAS_PRECUT},
    {0x04f9, 0x2004, "PT-2300", 112, 180, FLAG_RASTER_PACKBITS|FLAG_HAS_PRECUT},
    {0x04f9, 0x2007, "PT-2420PC", 128, 180, FLAG_RASTER_PACKBITS},
    {0x04f9, 0x2011, "PT-2450PC", 128, 180, FLAG_RASTER_PACKBITS},
    {0x04f9, 0x2019, "PT-1950", 112, 180, FLAG_RASTER_PACKBITS},
    {0x04f9, 0x201f, "PT-2700", 128, 180, FLAG_HAS_PRECUT},
    {0x04f9, 0x202c, "PT-1230PC", 128, 180, FLAG_NONE},
    {0x04f9, 0x202d, "PT-2430PC", 128, 180, FLAG_NONE},
    {0x04f9, 0x2030, "PT-1230PC (PLite Mode)", 128, 180, FLAG_PLITE},
    {0x04f9, 0x2031, "PT-2430PC (PLite Mode)", 128, 180, FLAG_PLITE},
    {0x04f9, 0x2041, "PT-2730", 128, 180, FLAG_NONE},
    {0x04f9, 0x205e, "PT-H500", 128, 180, FLAG_RASTER_PACKBITS},
    {0x04f9, 0x205f, "PT-E500", 128, 180, FLAG_RASTER_PACKBITS},
    {0x04f9, 0x2061, "PT-P700", 128, 180, FLAG_RASTER_PACKBITS|FLAG_P700_INIT|FLAG_HAS_PRECUT},
    {0x04f9, 0x2062, "PT-P750W", 128, 180, FLAG_RASTER_PACKBITS|FLAG_P700_INIT},
    {0x04f9, 0x2064, "PT-P700 (PLite Mode)", 128, 180, FLAG_PLITE},
    {0x04f9, 0x2065, "PT-P750W (PLite Mode)", 128, 180, FLAG_PLITE},
    {0x04f9, 0x20df, "PT-D410", 128, 180, FLAG_USE_INFO_CMD|FLAG_HAS_PRECUT|FLAG_D460BT_MAGIC},
    {0x04f9, 0x2073, "PT-D450", 128, 180, FLAG_USE_INFO_CMD},
    {0x04f9, 0x20e0, "PT-D460BT", 128, 180, FLAG_P700_INIT|FLAG_USE_INFO_CMD|FLAG_HAS_PRECUT|FLAG_D460BT_MAGIC},
    {0x04f9, 0x2074, "PT-D600", 128, 180, FLAG_RASTER_PACKBITS},
    {0x04f9, 0x20e1, "PT-D610BT", 128, 180, FLAG_P700_INIT|FLAG_USE_INFO_CMD|FLAG_HAS_PRECUT|FLAG_D460BT_MAGIC},
    {0x04f9, 0x20af, "PT-P710BT", 128, 180, FLAG_RASTER_PACKBITS|FLAG_HAS_PRECUT},
    {0x04f9, 0x2201, "PT-E310BT", 128, 180, FLAG_P700_INIT|FLAG_USE_INFO_CMD|FLAG_D460BT_MAGIC},
    {0, 0, "", 0, 0, 0}  // Terminator
};

// Tape information (ported from original library)
static const pt_tape_info tape_info[] = {
    { 4, 24, 0.5},   // 3.5 mm tape
    { 6, 32, 1.0},   // 6 mm tape
    { 9, 52, 1.0},   // 9 mm tape
    {12, 76, 2.0},   // 12 mm tape
    {18, 120, 3.0},  // 18 mm tape
    {24, 128, 3.0},  // 24 mm tape
    {36, 192, 4.5},  // 36 mm tape
    { 0, 0, 0.0}     // Terminator
};

static bool verbose_mode = false;

// Constructor
PtouchPrinter::PtouchPrinter() 
    : usb_host(nullptr), printer_device(nullptr), device_info(nullptr), 
      status(nullptr), tape_width_px(0), is_connected(false), is_initialized(false) {
    status = new ptouch_stat();
    memset(status, 0, sizeof(ptouch_stat));
}

// Destructor
PtouchPrinter::~PtouchPrinter() {
    disconnect();
    if (status) {
        delete status;
        status = nullptr;
    }
}

// Initialize USB host
bool PtouchPrinter::begin() {
    if (verbose_mode) Serial.println("Initializing USB Host...");
    
    if (!USB.begin()) {
        Serial.println("Failed to initialize USB");
        return false;
    }
    
    // Initialize USB Host
    usb_host = new USBHost();
    if (!usb_host) {
        Serial.println("Failed to create USB Host");
        return false;
    }
    
    if (verbose_mode) Serial.println("USB Host initialized successfully");
    return true;
}

// Detect Brother P-touch printer
bool PtouchPrinter::detectPrinter() {
    if (!usb_host) {
        Serial.println("USB Host not initialized");
        return false;
    }
    
    if (verbose_mode) Serial.println("Scanning for Brother P-touch printers...");
    
    // Scan for USB devices
    USBDevice *device = usb_host->getDevice(0);
    while (device) {
        uint16_t vid = device->getVendorID();
        uint16_t pid = device->getProductID();
        
        if (verbose_mode) {
            Serial.printf("Found USB device: VID=0x%04X, PID=0x%04X\n", vid, pid);
        }
        
        // Check if it's a Brother device
        if (vid == PTOUCH_VID) {
            // Find matching device in our supported list
            for (int i = 0; supported_devices[i].vid != 0; i++) {
                if (supported_devices[i].pid == pid) {
                    if (supported_devices[i].flags & FLAG_PLITE) {
                        Serial.printf("Found %s but it's in P-Lite mode (unsupported)\n", 
                                    supported_devices[i].name);
                        Serial.println("Switch to position E or press PLite button for 2 seconds");
                        return false;
                    }
                    
                    if (supported_devices[i].flags & FLAG_UNSUP_RASTER) {
                        Serial.printf("Found %s but it's currently unsupported\n", 
                                    supported_devices[i].name);
                        return false;
                    }
                    
                    Serial.printf("Found supported printer: %s\n", supported_devices[i].name);
                    printer_device = device;
                    device_info = const_cast<pt_dev_info*>(&supported_devices[i]);
                    return true;
                }
            }
            
            Serial.printf("Found Brother device (VID=0x%04X, PID=0x%04X) but it's not in our supported list\n", 
                        vid, pid);
        }
        
        device = usb_host->getDevice(device->getAddress() + 1);
    }
    
    if (verbose_mode) Serial.println("No supported Brother P-touch printer found");
    return false;
}

// Connect to the detected printer
bool PtouchPrinter::connect() {
    if (!printer_device || !device_info) {
        Serial.println("No printer detected. Call detectPrinter() first.");
        return false;
    }
    
    if (verbose_mode) Serial.println("Connecting to printer...");
    
    // Try to open the device
    if (!printer_device->open()) {
        Serial.println("Failed to open printer device");
        return false;
    }
    
    // Claim the interface
    if (!printer_device->claimInterface(0)) {
        Serial.println("Failed to claim printer interface");
        return false;
    }
    
    is_connected = true;
    
    // Initialize the printer
    if (initPrinter() != 0) {
        Serial.println("Failed to initialize printer");
        disconnect();
        return false;
    }
    
    is_initialized = true;
    
    if (verbose_mode) {
        Serial.printf("Successfully connected to %s\n", device_info->name);
        Serial.printf("Max width: %d px, DPI: %d\n", device_info->max_px, device_info->dpi);
    }
    
    return true;
}

// Disconnect from printer
void PtouchPrinter::disconnect() {
    if (printer_device && is_connected) {
        printer_device->releaseInterface(0);
        printer_device->close();
    }
    
    is_connected = false;
    is_initialized = false;
    printer_device = nullptr;
    device_info = nullptr;
}

// Send data to printer via USB
int PtouchPrinter::usbSend(uint8_t *data, size_t len) {
    if (!is_connected || !printer_device) {
        Serial.println("Printer not connected");
        return -1;
    }
    
    if (len > PTOUCH_MAX_PACKET_SIZE) {
        Serial.println("Data too large for single packet");
        return -1;
    }
    
    int result = printer_device->bulkTransfer(PTOUCH_BULK_OUT_ENDPOINT, data, len, 1000);
    
    if (result != (int)len) {
        Serial.printf("USB send failed: sent %d of %d bytes\n", result, len);
        return -1;
    }
    
    if (verbose_mode) {
        Serial.printf("Sent %d bytes to printer\n", len);
    }
    
    return 0;
}

// Receive data from printer via USB
int PtouchPrinter::usbReceive(uint8_t *data, size_t len) {
    if (!is_connected || !printer_device) {
        Serial.println("Printer not connected");
        return -1;
    }
    
    int result = printer_device->bulkTransfer(PTOUCH_BULK_IN_ENDPOINT, data, len, 1000);
    
    if (verbose_mode && result > 0) {
        Serial.printf("Received %d bytes from printer\n", result);
    }
    
    return result;
}

// Initialize printer (ported from original library)
int PtouchPrinter::initPrinter() {
    if (!device_info) return -1;
    
    // PT-P700 series initialization
    if (device_info->flags & FLAG_P700_INIT) {
        uint8_t init_cmd[] = {0x1b, 0x40};  // ESC @
        if (usbSend(init_cmd, sizeof(init_cmd)) != 0) {
            return -1;
        }
        delay(100);
    }
    
    // Send invalidate command
    uint8_t invalidate[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (usbSend(invalidate, sizeof(invalidate)) != 0) {
        return -1;
    }
    
    // Initialize command
    uint8_t init[] = {0x1b, 0x40};  // ESC @
    if (usbSend(init, sizeof(init)) != 0) {
        return -1;
    }
    
    delay(100);
    
    if (verbose_mode) {
        Serial.println("Printer initialized successfully");
    }
    
    return 0;
}

// Enable PackBits compression
int PtouchPrinter::enablePackBits() {
    uint8_t cmd[] = {0x1b, 0x69, 0x4b, 0x08};  // Enable compression
    return usbSend(cmd, sizeof(cmd));
}

// Send info command for newer printers
int PtouchPrinter::sendInfoCommand(int size_x) {
    uint8_t cmd[] = {0x1b, 0x69, 0x7a, 
                     (uint8_t)((size_x >> 0) & 0xff),
                     (uint8_t)((size_x >> 8) & 0xff),
                     (uint8_t)((size_x >> 16) & 0xff),
                     (uint8_t)((size_x >> 24) & 0xff),
                     0x00, 0x00, 0x00, 0x00, 0x00};
    return usbSend(cmd, sizeof(cmd));
}

// Send pre-cut command
int PtouchPrinter::sendPreCutCommand(int precut) {
    uint8_t cmd[] = {0x1b, 0x69, 0x4d, (uint8_t)(precut ? 0x40 : 0x00)};
    return usbSend(cmd, sizeof(cmd));
}

// Send magic commands for D460BT series
int PtouchPrinter::sendMagicCommands() {
    uint8_t magic1[] = {0x1b, 0x69, 0x61, 0x01};
    uint8_t magic2[] = {0x1b, 0x69, 0x21, 0x00};
    
    if (usbSend(magic1, sizeof(magic1)) != 0) return -1;
    if (usbSend(magic2, sizeof(magic2)) != 0) return -1;
    
    return 0;
}

// Start raster mode
int PtouchPrinter::rasterStart() {
    uint8_t cmd[] = {0x1b, 0x69, 0x52, 0x01};  // Enter raster mode
    return usbSend(cmd, sizeof(cmd));
}

// Send a raster line
int PtouchPrinter::sendRasterLine(uint8_t *data, size_t len) {
    if (len > 90) {  // Maximum raster line length
        Serial.println("Raster line too long");
        return -1;
    }
    
    uint8_t cmd[128];
    cmd[0] = 0x47;  // Raster line command
    cmd[1] = (uint8_t)len;
    memcpy(&cmd[2], data, len);
    
    return usbSend(cmd, len + 2);
}

// Set pixel in raster line (ported from original)
void PtouchPrinter::setRasterPixel(uint8_t* rasterline, size_t size, int pixel) {
    if (pixel < 0 || pixel >= (int)(size * 8)) {
        return;
    }
    rasterline[(size - 1) - (pixel / 8)] |= (uint8_t)(1 << (pixel % 8));
}

// Get printer information
const char* PtouchPrinter::getPrinterName() const {
    return device_info ? device_info->name : "Unknown";
}

int PtouchPrinter::getMaxWidth() const {
    return device_info ? device_info->max_px : 0;
}

int PtouchPrinter::getTapeWidth() const {
    return tape_width_px;
}

int PtouchPrinter::getDPI() const {
    return device_info ? device_info->dpi : 0;
}

// Get status from printer
bool PtouchPrinter::getStatus() {
    if (!is_connected) return false;
    
    uint8_t status_cmd[] = {0x1b, 0x69, 0x53};  // Status request
    if (usbSend(status_cmd, sizeof(status_cmd)) != 0) {
        return false;
    }
    
    uint8_t response[32];
    int received = usbReceive(response, sizeof(response));
    
    if (received == 32) {
        memcpy(status, response, 32);
        
        // Calculate tape width in pixels
        for (int i = 0; tape_info[i].mm != 0; i++) {
            if (tape_info[i].mm == status->media_width) {
                tape_width_px = tape_info[i].px;
                break;
            }
        }
        
        if (verbose_mode) {
            Serial.printf("Tape width: %d mm (%d px)\n", status->media_width, tape_width_px);
            Serial.printf("Media type: %s\n", getMediaType());
            Serial.printf("Tape color: %s\n", getTapeColor());
        }
        
        return true;
    }
    
    return false;
}

// Check if printer has error
bool PtouchPrinter::hasError() const {
    return status && (status->error != 0);
}

// Get error description
String PtouchPrinter::getErrorDescription() const {
    if (!status || status->error == 0) {
        return "No error";
    }
    
    // Error codes from original library
    switch (status->error) {
        case 0x01: return "No media";
        case 0x02: return "End of media";
        case 0x04: return "Cutter jam";
        case 0x08: return "Weak batteries";
        case 0x10: return "High voltage adapter";
        case 0x40: return "Replace media";
        case 0x80: return "Expansion buffer full";
        default: return "Unknown error: " + String(status->error, HEX);
    }
}

// Set verbose mode
void PtouchPrinter::setVerbose(bool verbose) {
    verbose_mode = verbose;
}

// Get supported devices list
const pt_dev_info* PtouchPrinter::getSupportedDevices() {
    return supported_devices;
}

// List supported printers
void PtouchPrinter::listSupportedPrinters() {
    Serial.println("Supported Brother P-touch printers:");
    for (int i = 0; supported_devices[i].vid != 0; i++) {
        if (!(supported_devices[i].flags & FLAG_PLITE)) {
            Serial.printf("  %s (VID: 0x%04X, PID: 0x%04X)\n", 
                        supported_devices[i].name,
                        supported_devices[i].vid, 
                        supported_devices[i].pid);
        }
    }
} 