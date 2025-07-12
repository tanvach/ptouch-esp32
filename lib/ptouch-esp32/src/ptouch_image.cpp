#include "ptouch_esp32.h"
#include <Arduino.h>

// Constructor - create empty bitmap
PtouchImage::PtouchImage(int w, int h) : width(w), height(h), owns_data(true) {
    int size = ((width + 7) / 8) * height;
    bitmap_data = new uint8_t[size];
    memset(bitmap_data, 0, size);
}

// Constructor - use existing data
PtouchImage::PtouchImage(const uint8_t *data, int w, int h, bool copy) 
    : width(w), height(h), owns_data(copy) {
    if (copy) {
        int size = ((width + 7) / 8) * height;
        bitmap_data = new uint8_t[size];
        memcpy(bitmap_data, data, size);
    } else {
        bitmap_data = const_cast<uint8_t*>(data);
    }
}

// Destructor
PtouchImage::~PtouchImage() {
    if (owns_data && bitmap_data) {
        delete[] bitmap_data;
    }
}

// Clear bitmap
void PtouchImage::clear() {
    if (bitmap_data) {
        int size = ((width + 7) / 8) * height;
        memset(bitmap_data, 0, size);
    }
}

// Set pixel
void PtouchImage::setPixel(int x, int y, bool black) {
    if (x < 0 || x >= width || y < 0 || y >= height || !bitmap_data) {
        return;
    }
    
    int byte_pos = (y * ((width + 7) / 8)) + (x / 8);
    int bit_pos = 7 - (x % 8);
    
    if (black) {
        bitmap_data[byte_pos] |= (1 << bit_pos);
    } else {
        bitmap_data[byte_pos] &= ~(1 << bit_pos);
    }
}

// Get pixel
bool PtouchImage::getPixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height || !bitmap_data) {
        return false;
    }
    
    int byte_pos = (y * ((width + 7) / 8)) + (x / 8);
    int bit_pos = 7 - (x % 8);
    
    return (bitmap_data[byte_pos] & (1 << bit_pos)) != 0;
}

// Draw line using Bresenham algorithm
void PtouchImage::drawLine(int x1, int y1, int x2, int y2, bool black) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        setPixel(x1, y1, black);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw rectangle outline
void PtouchImage::drawRect(int x, int y, int w, int h, bool black) {
    // Top and bottom lines
    for (int i = 0; i < w; i++) {
        setPixel(x + i, y, black);
        setPixel(x + i, y + h - 1, black);
    }
    
    // Left and right lines
    for (int i = 0; i < h; i++) {
        setPixel(x, y + i, black);
        setPixel(x + w - 1, y + i, black);
    }
}

// Fill rectangle
void PtouchImage::fillRect(int x, int y, int w, int h, bool black) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            setPixel(x + i, y + j, black);
        }
    }
}

// Draw character using simple 8x8 font
void PtouchImage::drawChar(int x, int y, char c, bool black) {
    // Simple 8x8 font data (same as in printing code)
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
    
    int font_index = 0;
    
    // Map character to font index
    if (c == ' ') font_index = 0;
    else if (c >= 'A' && c <= 'Z') font_index = c - 'A' + 1;
    else if (c >= 'a' && c <= 'z') font_index = c - 'a' + 1;
    else if (c >= '0' && c <= '9') font_index = c - '0' + 27;
    else font_index = 0; // Default to space
    
    // Draw character
    for (int row = 0; row < 8; row++) {
        uint8_t char_row = font_8x8[font_index][row];
        for (int col = 0; col < 8; col++) {
            if (char_row & (0x80 >> col)) {
                setPixel(x + col, y + row, black);
            }
        }
    }
}

// Draw text string
void PtouchImage::drawText(int x, int y, const char *text, bool black) {
    if (!text) return;
    
    int current_x = x;
    for (int i = 0; text[i] != '\0'; i++) {
        drawChar(current_x, y, text[i], black);
        current_x += 8; // Character width
    }
}

// Resize bitmap (simple implementation)
void PtouchImage::resize(int new_width, int new_height) {
    if (new_width <= 0 || new_height <= 0) return;
    
    int new_size = ((new_width + 7) / 8) * new_height;
    uint8_t *new_data = new uint8_t[new_size];
    memset(new_data, 0, new_size);
    
    // Copy existing data with simple scaling
    for (int y = 0; y < new_height && y < height; y++) {
        for (int x = 0; x < new_width && x < width; x++) {
            if (getPixel(x, y)) {
                int byte_pos = (y * ((new_width + 7) / 8)) + (x / 8);
                int bit_pos = 7 - (x % 8);
                new_data[byte_pos] |= (1 << bit_pos);
            }
        }
    }
    
    if (owns_data && bitmap_data) {
        delete[] bitmap_data;
    }
    
    bitmap_data = new_data;
    width = new_width;
    height = new_height;
    owns_data = true;
}

// Crop bitmap
PtouchImage* PtouchImage::crop(int x, int y, int w, int h) {
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || 
        x + w > width || y + h > height) {
        return nullptr;
    }
    
    PtouchImage *cropped = new PtouchImage(w, h);
    
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (getPixel(x + i, y + j)) {
                cropped->setPixel(i, j, true);
            }
        }
    }
    
    return cropped;
}

// Invert bitmap
void PtouchImage::invert() {
    if (!bitmap_data) return;
    
    int size = ((width + 7) / 8) * height;
    for (int i = 0; i < size; i++) {
        bitmap_data[i] = ~bitmap_data[i];
    }
} 