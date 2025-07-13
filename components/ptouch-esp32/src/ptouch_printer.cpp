#include "ptouch_esp32.h"
#include "esp_log.h"

static const char *TAG = "ptouch-printer";

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

// Constructor
PtouchPrinter::PtouchPrinter() 
    : client_hdl(nullptr), device_hdl(nullptr), device_info(nullptr), 
      status(nullptr), tape_width_px(0), is_connected(false), is_initialized(false), 
      verbose_mode(false), usb_host_installed(false), bulk_out_ep(0), bulk_in_ep(0) {
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

// USB Host client event callback
void PtouchPrinter::client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg) {
    PtouchPrinter *printer = static_cast<PtouchPrinter*>(arg);
    
    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            if (printer->verbose_mode) {
                ESP_LOGI(TAG, "New device connected");
            }
            break;
        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            if (printer->verbose_mode) {
                ESP_LOGI(TAG, "Device disconnected");
            }
            printer->is_connected = false;
            break;
        default:
            break;
    }
}

// Initialize USB host
bool PtouchPrinter::begin() {
    if (verbose_mode) ESP_LOGI(TAG, "Initializing USB Host...");
    
    // Install USB Host driver
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    
    esp_err_t err = usb_host_install(&host_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install USB Host driver: %s", esp_err_to_name(err));
        return false;
    }
    
    usb_host_installed = true;
    
    // Register USB Host client
    const usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = client_event_cb,
            .callback_arg = this,
        }
    };
    
    err = usb_host_client_register(&client_config, &client_hdl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register USB Host client: %s", esp_err_to_name(err));
        usb_host_uninstall();
        usb_host_installed = false;
        return false;
    }
    
    if (verbose_mode) ESP_LOGI(TAG, "USB Host initialized successfully");
    return true;
}

// Detect Brother P-touch printer
bool PtouchPrinter::detectPrinter() {
    if (!client_hdl) {
        ESP_LOGE(TAG, "USB Host client not initialized");
        return false;
    }
    
    if (verbose_mode) ESP_LOGI(TAG, "Scanning for Brother P-touch printers...");
    
    // Get list of connected devices
    uint8_t dev_addr_list[10];
    int num_dev = 0;
    esp_err_t err = usb_host_device_addr_list_fill(sizeof(dev_addr_list), dev_addr_list, &num_dev);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get device address list: %s", esp_err_to_name(err));
        return false;
    }
    
    if (verbose_mode) ESP_LOGI(TAG, "Found %d USB devices", num_dev);
    
    // Check each device
    for (int i = 0; i < num_dev; i++) {
        usb_device_handle_t dev_hdl;
        err = usb_host_device_open(client_hdl, dev_addr_list[i], &dev_hdl);
        if (err != ESP_OK) {
            continue;
        }
        
        // Get device descriptor
        const usb_device_desc_t *dev_desc;
        err = usb_host_get_device_descriptor(dev_hdl, &dev_desc);
        if (err != ESP_OK) {
            usb_host_device_close(client_hdl, dev_hdl);
            continue;
        }
        
        uint16_t vid = dev_desc->idVendor;
        uint16_t pid = dev_desc->idProduct;
        
        if (verbose_mode) {
            ESP_LOGI(TAG, "Found USB device: VID=0x%04X, PID=0x%04X", vid, pid);
        }
        
        // Check if it's a Brother device
        if (vid == PTOUCH_VID) {
            // Find matching device in our supported list
            for (int j = 0; supported_devices[j].vid != 0; j++) {
                if (supported_devices[j].pid == pid) {
                    if (supported_devices[j].flags & FLAG_PLITE) {
                        ESP_LOGW(TAG, "Found %s but it's in P-Lite mode (unsupported)", 
                                supported_devices[j].name);
                        ESP_LOGW(TAG, "Switch to position E or press PLite button for 2 seconds");
                        usb_host_device_close(client_hdl, dev_hdl);
                        return false;
                    }
                    
                    if (supported_devices[j].flags & FLAG_UNSUP_RASTER) {
                        ESP_LOGW(TAG, "Found %s but it's currently unsupported", 
                                supported_devices[j].name);
                        usb_host_device_close(client_hdl, dev_hdl);
                        return false;
                    }
                    
                    ESP_LOGI(TAG, "Found supported printer: %s", supported_devices[j].name);
                    device_hdl = dev_hdl;
                    device_info = const_cast<pt_dev_info*>(&supported_devices[j]);
                    return true;
                }
            }
            
            ESP_LOGW(TAG, "Found Brother device (VID=0x%04X, PID=0x%04X) but it's not in our supported list", 
                    vid, pid);
        }
        
        usb_host_device_close(client_hdl, dev_hdl);
    }
    
    if (verbose_mode) ESP_LOGI(TAG, "No supported Brother P-touch printer found");
    return false;
}

// Connect to the detected printer
bool PtouchPrinter::connect() {
    if (!device_hdl || !device_info) {
        ESP_LOGE(TAG, "No printer detected. Call detectPrinter() first.");
        return false;
    }
    
    if (verbose_mode) ESP_LOGI(TAG, "Connecting to printer...");
    
    // Claim the interface
    if (!claimInterface()) {
        ESP_LOGE(TAG, "Failed to claim printer interface");
        return false;
    }
    
    // Get endpoint addresses
    if (!getEndpoints()) {
        ESP_LOGE(TAG, "Failed to get USB endpoints");
        releaseInterface();
        return false;
    }
    
    is_connected = true;
    
    // Initialize the printer
    if (initPrinter() != 0) {
        ESP_LOGE(TAG, "Failed to initialize printer");
        disconnect();
        return false;
    }
    
    is_initialized = true;
    
    if (verbose_mode) {
        ESP_LOGI(TAG, "Successfully connected to %s", device_info->name);
        ESP_LOGI(TAG, "Max width: %d px, DPI: %d", device_info->max_px, device_info->dpi);
    }
    
    return true;
}

// Claim USB interface
bool PtouchPrinter::claimInterface() {
    if (!device_hdl) return false;
    
    esp_err_t err = usb_host_interface_claim(client_hdl, device_hdl, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to claim interface: %s", esp_err_to_name(err));
        return false;
    }
    
    return true;
}

// Release USB interface
void PtouchPrinter::releaseInterface() {
    if (device_hdl) {
        usb_host_interface_release(client_hdl, device_hdl, 0);
    }
}

// Get USB endpoint addresses
bool PtouchPrinter::getEndpoints() {
    if (!device_hdl) return false;
    
    // Get configuration descriptor
    const usb_config_desc_t *config_desc;
    esp_err_t err = usb_host_get_active_config_descriptor(device_hdl, &config_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get config descriptor: %s", esp_err_to_name(err));
        return false;
    }
    
    // Parse interfaces and endpoints
    int offset = 0;
    const usb_intf_desc_t *intf_desc = (const usb_intf_desc_t*)usb_parse_next_descriptor_of_type(
        (const usb_standard_desc_t*)config_desc, config_desc->wTotalLength, USB_W_VALUE_DT_INTERFACE, &offset);
    
    if (!intf_desc) {
        ESP_LOGE(TAG, "No interface descriptor found");
        return false;
    }
    
    // Find bulk endpoints
    for (int i = 0; i < intf_desc->bNumEndpoints; i++) {
        const usb_ep_desc_t *ep_desc = (const usb_ep_desc_t*)usb_parse_next_descriptor_of_type(
            (const usb_standard_desc_t*)config_desc, config_desc->wTotalLength, USB_W_VALUE_DT_ENDPOINT, &offset);
        
        if (!ep_desc) continue;
        
        if ((ep_desc->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_BULK) {
            if (ep_desc->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) {
                // IN endpoint
                bulk_in_ep = ep_desc->bEndpointAddress;
                if (verbose_mode) ESP_LOGI(TAG, "Found bulk IN endpoint: 0x%02X", bulk_in_ep);
            } else {
                // OUT endpoint
                bulk_out_ep = ep_desc->bEndpointAddress;
                if (verbose_mode) ESP_LOGI(TAG, "Found bulk OUT endpoint: 0x%02X", bulk_out_ep);
            }
        }
    }
    
    return (bulk_in_ep != 0 && bulk_out_ep != 0);
}

// Disconnect from printer
void PtouchPrinter::disconnect() {
    if (is_connected) {
        releaseInterface();
        is_connected = false;
        is_initialized = false;
    }
    
    if (device_hdl) {
        usb_host_device_close(client_hdl, device_hdl);
        device_hdl = nullptr;
    }
    
    if (client_hdl) {
        usb_host_client_deregister(client_hdl);
        client_hdl = nullptr;
    }
    
    if (usb_host_installed) {
        usb_host_uninstall();
        usb_host_installed = false;
    }
    
    device_info = nullptr;
    if (status) {
        memset(status, 0, sizeof(ptouch_stat));
    }
    tape_width_px = 0;
    
    if (verbose_mode) {
        ESP_LOGI(TAG, "Disconnected from printer");
    }
}

// Send data to printer via USB
int PtouchPrinter::usbSend(uint8_t *data, size_t len) {
    if (!is_connected || !device_hdl) {
        ESP_LOGE(TAG, "Printer not connected");
        return -1;
    }
    
    if (len > PTOUCH_MAX_PACKET_SIZE) {
        ESP_LOGE(TAG, "Data too large for single packet");
        return -1;
    }
    
    // Create USB transfer
    usb_transfer_t *transfer = nullptr;
    esp_err_t err = usb_host_transfer_alloc(len, 0, &transfer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to allocate USB transfer: %s", esp_err_to_name(err));
        return -1;
    }
    
    // Fill transfer
    transfer->device_handle = device_hdl;
    transfer->bEndpointAddress = bulk_out_ep;
    transfer->callback = nullptr;
    transfer->context = this;
    transfer->num_bytes = len;
    memcpy(transfer->data_buffer, data, len);
    
    // Submit transfer
    err = usb_host_transfer_submit(transfer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to submit USB transfer: %s", esp_err_to_name(err));
        usb_host_transfer_free(transfer);
        return -1;
    }
    
    // Wait for completion (simple blocking implementation)
    int attempts = 1000;
    while (transfer->status == USB_TRANSFER_STATUS_COMPLETED + 1 && attempts-- > 0) {
        usb_host_client_handle_events(client_hdl, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    int result = -1;
    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        result = transfer->actual_num_bytes;
        if (verbose_mode) {
            ESP_LOGI(TAG, "Sent %d bytes to printer", result);
        }
    } else {
        ESP_LOGE(TAG, "USB transfer failed with status: %d", transfer->status);
    }
    
    usb_host_transfer_free(transfer);
    return result;
}

// Receive data from printer via USB
int PtouchPrinter::usbReceive(uint8_t *data, size_t len) {
    if (!is_connected || !device_hdl) {
        ESP_LOGE(TAG, "Printer not connected");
        return -1;
    }
    
    // Create USB transfer
    usb_transfer_t *transfer = nullptr;
    esp_err_t err = usb_host_transfer_alloc(len, 0, &transfer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to allocate USB transfer: %s", esp_err_to_name(err));
        return -1;
    }
    
    // Fill transfer
    transfer->device_handle = device_hdl;
    transfer->bEndpointAddress = bulk_in_ep;
    transfer->callback = nullptr;
    transfer->context = this;
    transfer->num_bytes = len;
    
    // Submit transfer
    err = usb_host_transfer_submit(transfer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to submit USB transfer: %s", esp_err_to_name(err));
        usb_host_transfer_free(transfer);
        return -1;
    }
    
    // Wait for completion
    int attempts = 1000;
    while (transfer->status == USB_TRANSFER_STATUS_COMPLETED + 1 && attempts-- > 0) {
        usb_host_client_handle_events(client_hdl, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    int result = -1;
    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
        result = transfer->actual_num_bytes;
        memcpy(data, transfer->data_buffer, result);
        if (verbose_mode && result > 0) {
            ESP_LOGI(TAG, "Received %d bytes from printer", result);
        }
    }
    
    usb_host_transfer_free(transfer);
    return result;
}

// Initialize printer (ported from original library)
int PtouchPrinter::initPrinter() {
    if (!device_info) return -1;
    
    // PT-P700 series initialization
    if (device_info->flags & FLAG_P700_INIT) {
        uint8_t init_cmd[] = {0x1b, 0x40};  // ESC @
        if (usbSend(init_cmd, sizeof(init_cmd)) < 0) {
            return -1;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Send 102-byte invalidate command (100 zeros + ESC @) - FIXED
    uint8_t invalidate_cmd[102];
    memset(invalidate_cmd, 0, 100);  // First 100 bytes are zero
    invalidate_cmd[100] = 0x1b;      // ESC
    invalidate_cmd[101] = 0x40;      // @
    if (usbSend(invalidate_cmd, sizeof(invalidate_cmd)) < 0) {
        return -1;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    if (verbose_mode) {
        ESP_LOGI(TAG, "Printer initialized successfully");
    }
    
    return 0;
}

// Enable PackBits compression
int PtouchPrinter::enablePackBits() {
    uint8_t cmd[] = {0x4d, 0x02};  // 4D 02 = enable packbits compression mode - FIXED
    return (usbSend(cmd, sizeof(cmd)) > 0) ? 0 : -1;
}

// Send info command for newer printers
int PtouchPrinter::sendInfoCommand(int size_x) {
    uint8_t cmd[] = {0x1b, 0x69, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    // Set media width (n3) - FIXED
    cmd[5] = status ? status->media_width : 0;
    
    // Set raster number (n5-n8) - FIXED positioning
    cmd[7] = (uint8_t)(size_x & 0xff);
    cmd[8] = (uint8_t)((size_x >> 8) & 0xff);
    cmd[9] = (uint8_t)((size_x >> 16) & 0xff);
    cmd[10] = (uint8_t)((size_x >> 24) & 0xff);
    
    // D460BT magic flag handling - FIXED
    if (device_info && (device_info->flags & FLAG_D460BT_MAGIC)) {
        cmd[11] = 0x02;  // n9 is set to 2 for D460BT
    }
    
    return (usbSend(cmd, sizeof(cmd) - 1) > 0) ? 0 : -1;  // sizeof(cmd) - 1 like original
}

// Send pre-cut command
int PtouchPrinter::sendPreCutCommand(int precut) {
    uint8_t cmd[] = {0x1b, 0x69, 0x4d, (uint8_t)(precut ? 0x40 : 0x00)};
    return (usbSend(cmd, sizeof(cmd)) > 0) ? 0 : -1;
}

// Send magic commands for D460BT series
int PtouchPrinter::sendMagicCommands() {
    // Send D460BT chain command first - FIXED
    uint8_t chain_cmd[] = {0x1b, 0x69, 0x4b, 0x00};
    if (usbSend(chain_cmd, sizeof(chain_cmd)) < 0) return -1;
    
    // Send D460BT magic command - FIXED
    uint8_t magic_cmd[] = {0x1b, 0x69, 0x64, 0x0e, 0x00, 0x4d, 0x00};
    if (usbSend(magic_cmd, sizeof(magic_cmd)) < 0) return -1;
    
    return 0;
}

// Start raster mode
int PtouchPrinter::rasterStart() {
    // Different commands for different printer series - FIXED
    if (device_info && (device_info->flags & FLAG_P700_INIT)) {
        uint8_t cmd[] = {0x1b, 0x69, 0x61, 0x01};  // Switch mode for P700 series
        return (usbSend(cmd, sizeof(cmd)) > 0) ? 0 : -1;
    } else {
        uint8_t cmd[] = {0x1b, 0x69, 0x52, 0x01};  // Select graphics transfer mode = Raster
        return (usbSend(cmd, sizeof(cmd)) > 0) ? 0 : -1;
    }
}

// Send a raster line
int PtouchPrinter::sendRasterLine(uint8_t *data, size_t len) {
    if (len > (size_t)(device_info->max_px / 8)) {  // Check against max pixels
        ESP_LOGE(TAG, "Raster line too long");
        return -1;
    }
    
    uint8_t cmd[128];
    cmd[0] = 0x47;  // Raster line command
    
    // Handle PackBits compression - FIXED
    if (device_info && (device_info->flags & FLAG_RASTER_PACKBITS)) {
        // Fake compression by encoding a single uncompressed run
        cmd[1] = (uint8_t)(len + 1);
        cmd[2] = 0;
        cmd[3] = (uint8_t)(len - 1);
        memcpy(&cmd[4], data, len);
        return (usbSend(cmd, len + 4) > 0) ? 0 : -1;
    } else {
        // Uncompressed mode
        cmd[1] = (uint8_t)len;
        cmd[2] = 0;
        memcpy(&cmd[3], data, len);
        return (usbSend(cmd, len + 3) > 0) ? 0 : -1;
    }
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
    if (usbSend(status_cmd, sizeof(status_cmd)) < 0) {
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
            ESP_LOGI(TAG, "Tape width: %d mm (%d px)", status->media_width, tape_width_px);
            ESP_LOGI(TAG, "Media type: %s", getMediaType());
            ESP_LOGI(TAG, "Tape color: %s", getTapeColor());
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
const char* PtouchPrinter::getErrorDescription() const {
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
        default: return "Unknown error";
    }
}

// Set verbose mode
void PtouchPrinter::setVerbose(bool verbose) {
    this->verbose_mode = verbose;
}

// Get supported devices list
const pt_dev_info* PtouchPrinter::getSupportedDevices() {
    return supported_devices;
}

// List supported printers
void PtouchPrinter::listSupportedPrinters() {
    ESP_LOGI(TAG, "Supported Brother P-touch printers:");
    for (int i = 0; supported_devices[i].vid != 0; i++) {
        if (!(supported_devices[i].flags & FLAG_PLITE)) {
            ESP_LOGI(TAG, "  %s (VID: 0x%04X, PID: 0x%04X)", 
                    supported_devices[i].name,
                    supported_devices[i].vid, 
                    supported_devices[i].pid);
        }
    }
}

// Note: Media type, tape color, and text color functions are implemented in ptouch_utils.cpp

// Note: Page control methods (setPageFlags, feedPaper, cutPaper, finalizePrint) 
// are implemented in ptouch_printing.cpp 