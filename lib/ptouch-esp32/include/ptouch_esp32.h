#ifndef PTOUCH_ESP32_H
#define PTOUCH_ESP32_H

#include <Arduino.h>
#include <stdint.h>
#include <USB.h>

// Brother P-touch printer constants
#define PTOUCH_VID                 0x04F9
#define PTOUCH_MAX_PACKET_SIZE     128
#define PTOUCH_BULK_OUT_ENDPOINT   0x02
#define PTOUCH_BULK_IN_ENDPOINT    0x81

// Printer flags (ported from original library)
#define FLAG_NONE                  (0)
#define FLAG_UNSUP_RASTER          (1 << 0)
#define FLAG_RASTER_PACKBITS       (1 << 1)
#define FLAG_PLITE                 (1 << 2)
#define FLAG_P700_INIT             (1 << 3)
#define FLAG_USE_INFO_CMD          (1 << 4)
#define FLAG_HAS_PRECUT            (1 << 5)
#define FLAG_D460BT_MAGIC          (1 << 6)

// Page flags for printing
typedef enum {
    FEED_NONE    = 0x0,
    FEED_SMALL   = 0x08,
    FEED_MEDIUM  = 0x0c,
    FEED_LARGE   = 0x1a,
    AUTO_CUT     = (1 << 6),
    MIRROR       = (1 << 7),
} pt_page_flags;

// Tape information structure
struct pt_tape_info {
    uint8_t mm;          // Tape width in mm
    uint16_t px;         // Printing area in px
    double margins;      // Default tape margins in mm
};

// Device information structure
struct pt_dev_info {
    int vid;             // USB vendor ID
    int pid;             // USB product ID
    const char *name;    // Device name
    int max_px;          // Maximum pixel width
    int dpi;             // Dots per inch
    int flags;           // Device flags
};

// Printer status structure (packed for compatibility)
struct __attribute__((packed, aligned(4))) ptouch_stat {
    uint8_t printheadmark;    // 0x80
    uint8_t size;             // 0x20
    uint8_t brother_code;     // "B"
    uint8_t series_code;      // "0"
    uint8_t model;
    uint8_t country;          // "0"
    uint16_t reserved_1;
    uint16_t error;           // Error codes
    uint8_t media_width;      // Tape width in mm
    uint8_t media_type;       // Media type
    uint8_t ncol;             // 0
    uint8_t fonts;            // 0
    uint8_t jp_fonts;         // 0
    uint8_t mode;
    uint8_t density;          // 0
    uint8_t media_len;        // Tape length
    uint8_t status_type;      // Status type
    uint8_t phase_type;
    uint16_t phase_number;
    uint8_t notif_number;
    uint8_t exp;              // 0
    uint8_t tape_color;       // Tape color
    uint8_t text_color;       // Text color
    uint32_t hw_setting;
    uint16_t reserved_2;
};

// Main printer device class
class PtouchPrinter {
private:
    void *usb_host;  // Placeholder for USB Host (not available in Arduino framework)
    pt_dev_info *device_info;
    ptouch_stat *status;
    uint16_t tape_width_px;
    bool is_connected;
    bool is_initialized;
    bool verbose_mode;
    
    // USB communication methods
    int usbSend(uint8_t *data, size_t len);
    int usbReceive(uint8_t *data, size_t len);
    
    // Printer initialization methods
    int initPrinter();
    int sendMagicCommands();
    int enablePackBits();
    int sendInfoCommand(int size_x);
    int sendPreCutCommand(int precut);
    
    // Raster data methods
    int rasterStart();
    int sendRasterLine(uint8_t *data, size_t len);
    void setRasterPixel(uint8_t* rasterline, size_t size, int pixel);

public:
    PtouchPrinter();
    ~PtouchPrinter();
    
    // Device management
    bool begin();
    bool detectPrinter();
    bool connect();
    void disconnect();
    bool isConnected() const { return is_connected; }
    
    // Printer information
    const char* getPrinterName() const;
    int getMaxWidth() const;
    int getTapeWidth() const;
    int getDPI() const;
    
    // Status and diagnostics
    bool getStatus();
    const char* getMediaType() const;
    const char* getTapeColor() const;
    const char* getTextColor() const;
    bool hasError() const;
    String getErrorDescription() const;
    
    // Printing methods
    bool printImage(const uint8_t *imageData, int width, int height, bool chain = false);
    bool printBitmap(const uint8_t *bitmap, int width, int height, bool chain = false);
    bool printText(const char *text, int fontSize = 0, bool chain = false);
    
    // Utility methods
    void setVerbose(bool verbose);
    void listSupportedPrinters();
    static const pt_dev_info* getSupportedDevices();
    
    // Page control
    bool setPageFlags(pt_page_flags flags);
    bool feedPaper(int amount);
    bool cutPaper();
    bool finalizePrint(bool chain = false);
};

// Image processing class for simple bitmap operations
class PtouchImage {
private:
    uint8_t *bitmap_data;
    int width;
    int height;
    bool owns_data;
    
public:
    PtouchImage(int w, int h);
    PtouchImage(const uint8_t *data, int w, int h, bool copy = true);
    ~PtouchImage();
    
    // Basic operations
    void clear();
    void setPixel(int x, int y, bool black = true);
    bool getPixel(int x, int y) const;
    void drawLine(int x1, int y1, int x2, int y2, bool black = true);
    void drawRect(int x, int y, int w, int h, bool black = true);
    void fillRect(int x, int y, int w, int h, bool black = true);
    
    // Text rendering (basic)
    void drawChar(int x, int y, char c, bool black = true);
    void drawText(int x, int y, const char *text, bool black = true);
    
    // Data access
    const uint8_t* getData() const { return bitmap_data; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Utility
    void resize(int new_width, int new_height);
    PtouchImage* crop(int x, int y, int w, int h);
    void invert();
};

// Utility functions
const char* pt_mediatype_string(uint8_t media_type);
const char* pt_tapecolor_string(uint8_t tape_color);
const char* pt_textcolor_string(uint8_t text_color);

#endif // PTOUCH_ESP32_H 