#include "ptouch_esp32.h"
#include <Arduino.h>

// Print bitmap image to printer
bool PtouchPrinter::printBitmap(const uint8_t *bitmap, int width, int height, bool chain) {
    if (!is_connected || !is_initialized) {
        Serial.println("Printer not connected or initialized");
        return false;
    }
    
    if (!bitmap) {
        Serial.println("Invalid bitmap data");
        return false;
    }
    
    // Get printer status and tape width
    if (!getStatus()) {
        Serial.println("Failed to get printer status");
        return false;
    }
    
    int max_pixels = getMaxWidth();
    int tape_width = getTapeWidth();
    
    if (height > tape_width) {
        Serial.printf("Image too tall: %d px, max: %d px\n", height, tape_width);
        return false;
    }
    
    if (verbose_mode) {
        Serial.printf("Printing bitmap: %d x %d pixels\n", width, height);
        Serial.printf("Tape width: %d px, Max width: %d px\n", tape_width, max_pixels);
    }
    
    // Calculate centering offset
    int offset = (max_pixels / 2) - (height / 2);
    
    // Enable compression if supported
    if (device_info->flags & FLAG_RASTER_PACKBITS) {
        if (enablePackBits() != 0) {
            Serial.println("Failed to enable PackBits compression");
            return false;
        }
    }
    
    // Start raster mode
    if (rasterStart() != 0) {
        Serial.println("Failed to start raster mode");
        return false;
    }
    
    // Send info command for newer printers
    if (device_info->flags & FLAG_USE_INFO_CMD) {
        if (sendInfoCommand(width) != 0) {
            Serial.println("Failed to send info command");
            return false;
        }
    }
    
    // Send magic commands for D460BT series
    if (device_info->flags & FLAG_D460BT_MAGIC) {
        if (sendMagicCommands() != 0) {
            Serial.println("Failed to send magic commands");
            return false;
        }
    }
    
    // Send pre-cut command if supported
    if (device_info->flags & FLAG_HAS_PRECUT) {
        if (sendPreCutCommand(1) != 0) {
            Serial.println("Failed to send precut command");
            return false;
        }
    }
    
    // Send raster data line by line
    uint8_t raster_line[max_pixels / 8];
    
    for (int x = 0; x < width; x++) {
        memset(raster_line, 0, sizeof(raster_line));
        
        // Build raster line from bitmap
        for (int y = 0; y < height; y++) {
            // Calculate bitmap byte position
            int byte_pos = (y * ((width + 7) / 8)) + (x / 8);
            int bit_pos = 7 - (x % 8);
            
            // Check if pixel is set (black)
            if (bitmap[byte_pos] & (1 << bit_pos)) {
                setRasterPixel(raster_line, sizeof(raster_line), offset + (height - 1 - y));
            }
        }
        
        // Send raster line
        if (sendRasterLine(raster_line, max_pixels / 8) != 0) {
            Serial.printf("Failed to send raster line %d\n", x);
            return false;
        }
    }
    
    // Finalize print job
    if (finalizePrint(chain) != 0) {
        Serial.println("Failed to finalize print");
        return false;
    }
    
    if (verbose_mode) {
        Serial.println("Print job completed successfully");
    }
    
    return true;
}

// Print image data (assumes image is already processed to monochrome)
bool PtouchPrinter::printImage(const uint8_t *imageData, int width, int height, bool chain) {
    // For now, delegate to printBitmap
    return printBitmap(imageData, width, height, chain);
}

// Simple text printing using built-in 8x8 font
bool PtouchPrinter::printText(const char *text, int fontSize, bool chain) {
    if (!text || strlen(text) == 0) {
        Serial.println("Invalid text input");
        return false;
    }
    
    if (verbose_mode) {
        Serial.printf("Printing text: '%s'\n", text);
    }
    
    // Create a simple bitmap for text
    // This is a basic implementation - in a real system you'd want better font rendering
    int char_width = 8;
    int char_height = 8;
    int text_len = strlen(text);
    
    // Calculate image dimensions
    int image_width = text_len * char_width;
    int image_height = char_height;
    
    // Create bitmap buffer
    int bitmap_size = ((image_width + 7) / 8) * image_height;
    uint8_t *bitmap = new uint8_t[bitmap_size];
    memset(bitmap, 0, bitmap_size);
    
    // Simple 8x8 font (A-Z, 0-9, space)
    // This is a very basic implementation
    const uint8_t font_8x8[][8] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
        {0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // A
        {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, // B
        {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // C
        {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // D
        {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, // E
        {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00}, // F
        {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00}, // G
        {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // H
        {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // I
        {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00}, // J
        {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, // K
        {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // L
        {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // M
        {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}, // N
        {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // O
        {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}, // P
        {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x0E, 0x00}, // Q
        {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00}, // R
        {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, // S
        {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T
        {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // U
        {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // V
        {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // W
        {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00}, // X
        {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, // Y
        {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00}, // Z
        {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, // 0
        {0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E, 0x00}, // 1
        {0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00}, // 2
        {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, // 3
        {0x06, 0x0E, 0x1E, 0x66, 0x7F, 0x06, 0x06, 0x00}, // 4
        {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 5
        {0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00}, // 6
        {0x7E, 0x66, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00}, // 7
        {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, // 8
        {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00}, // 9
    };
    
    // Render text to bitmap
    for (int i = 0; i < text_len; i++) {
        char c = text[i];
        int font_index = 0;
        
        // Map character to font index
        if (c == ' ') font_index = 0;
        else if (c >= 'A' && c <= 'Z') font_index = c - 'A' + 1;
        else if (c >= 'a' && c <= 'z') font_index = c - 'a' + 1;
        else if (c >= '0' && c <= '9') font_index = c - '0' + 27;
        else font_index = 0; // Default to space
        
        // Copy character bitmap
        for (int y = 0; y < char_height; y++) {
            uint8_t char_row = font_8x8[font_index][y];
            for (int x = 0; x < char_width; x++) {
                if (char_row & (0x80 >> x)) {
                    int bitmap_x = i * char_width + x;
                    int bitmap_y = y;
                    int byte_pos = (bitmap_y * ((image_width + 7) / 8)) + (bitmap_x / 8);
                    int bit_pos = 7 - (bitmap_x % 8);
                    
                    if (byte_pos < bitmap_size) {
                        bitmap[byte_pos] |= (1 << bit_pos);
                    }
                }
            }
        }
    }
    
    // Print the bitmap
    bool result = printBitmap(bitmap, image_width, image_height, chain);
    
    // Clean up
    delete[] bitmap;
    
    return result;
}

// Set page flags for printing
bool PtouchPrinter::setPageFlags(pt_page_flags flags) {
    if (!is_connected) {
        Serial.println("Printer not connected");
        return false;
    }
    
    uint8_t cmd[] = {0x1b, 0x69, 0x4D, (uint8_t)flags};
    return usbSend(cmd, sizeof(cmd)) == 0;
}

// Feed paper
bool PtouchPrinter::feedPaper(int amount) {
    if (!is_connected) {
        Serial.println("Printer not connected");
        return false;
    }
    
    uint8_t cmd[] = {0x1b, 0x69, 0x64, (uint8_t)amount};
    return usbSend(cmd, sizeof(cmd)) == 0;
}

// Cut paper
bool PtouchPrinter::cutPaper() {
    if (!is_connected) {
        Serial.println("Printer not connected");
        return false;
    }
    
    uint8_t cmd[] = {0x1b, 0x69, 0x4B, 0x08};
    return usbSend(cmd, sizeof(cmd)) == 0;
}

// Finalize print job
bool PtouchPrinter::finalizePrint(bool chain) {
    if (!is_connected) {
        Serial.println("Printer not connected");
        return false;
    }
    
    // Form feed and print
    uint8_t cmd[] = {0x1a};
    if (usbSend(cmd, sizeof(cmd)) != 0) {
        return false;
    }
    
    // If not chaining, send final form feed
    if (!chain) {
        uint8_t ff_cmd[] = {0x1b, 0x69, 0x41, 0x01};
        if (usbSend(ff_cmd, sizeof(ff_cmd)) != 0) {
            return false;
        }
    }
    
    return true;
} 