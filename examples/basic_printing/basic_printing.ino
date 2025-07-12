/*
 * Basic P-touch Printing Example
 * 
 * This example demonstrates basic usage of the PTouch ESP32 library
 * for printing text labels on Brother P-touch printers.
 * 
 * Hardware:
 * - ESP32-S3-DevKitC-1 or compatible
 * - Brother P-touch printer (connected via USB)
 * 
 * Required Libraries:
 * - ptouch-esp32 (this library)
 */

#include "ptouch_esp32.h"

// Create printer instance
PtouchPrinter printer;

void setup() {
    Serial.begin(115200);
    Serial.println("P-touch ESP32 Basic Printing Example");
    
    // Enable verbose output for debugging
    printer.setVerbose(true);
    
    // Initialize USB host
    if (!printer.begin()) {
        Serial.println("Failed to initialize USB Host");
        return;
    }
    
    Serial.println("USB Host initialized");
    
    // Detect printer
    if (!printer.detectPrinter()) {
        Serial.println("No supported printer found");
        Serial.println("Make sure your printer is:");
        Serial.println("1. Powered on");
        Serial.println("2. Connected via USB");
        Serial.println("3. Not in P-Lite mode");
        return;
    }
    
    Serial.println("Printer detected!");
    
    // Connect to printer
    if (!printer.connect()) {
        Serial.println("Failed to connect to printer");
        return;
    }
    
    Serial.println("Connected to printer successfully");
    
    // Print basic information
    Serial.printf("Printer: %s\n", printer.getPrinterName());
    Serial.printf("Max width: %d pixels\n", printer.getMaxWidth());
    Serial.printf("DPI: %d\n", printer.getDPI());
    
    // Get printer status
    if (printer.getStatus()) {
        Serial.printf("Tape width: %d pixels\n", printer.getTapeWidth());
        Serial.printf("Media type: %s\n", printer.getMediaType());
        Serial.printf("Tape color: %s\n", printer.getTapeColor());
        
        if (printer.hasError()) {
            Serial.printf("Printer error: %s\n", printer.getErrorDescription().c_str());
        }
    }
    
    // Print some test labels
    Serial.println("\nPrinting test labels...");
    
    // Simple text
    if (printer.printText("Hello World!")) {
        Serial.println("✓ Printed: Hello World!");
    } else {
        Serial.println("✗ Failed to print text");
    }
    
    delay(2000);
    
    // Text with numbers
    if (printer.printText("ESP32 2024")) {
        Serial.println("✓ Printed: ESP32 2024");
    } else {
        Serial.println("✗ Failed to print text");
    }
    
    delay(2000);
    
    // Create a simple bitmap pattern
    printBitmapPattern();
    
    Serial.println("\nPrinting complete!");
    Serial.println("You can now disconnect the printer or print more labels.");
}

void loop() {
    // Check printer status periodically
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000) { // Every 10 seconds
        lastCheck = millis();
        
        if (printer.isConnected()) {
            if (printer.getStatus()) {
                Serial.println("Printer is ready");
                
                // Print current time as a label
                String timeLabel = "Time: " + String(millis() / 1000) + "s";
                if (printer.printText(timeLabel.c_str())) {
                    Serial.println("✓ Printed time label");
                }
            } else {
                Serial.println("Printer connection lost");
            }
        } else {
            Serial.println("Printer not connected");
        }
    }
    
    delay(1000);
}

void printBitmapPattern() {
    Serial.println("Creating bitmap pattern...");
    
    // Create a simple 64x16 bitmap with a pattern
    const int width = 64;
    const int height = 16;
    const int bitmap_size = ((width + 7) / 8) * height;
    
    uint8_t bitmap[bitmap_size];
    memset(bitmap, 0, bitmap_size);
    
    // Create a simple pattern
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create a checkerboard pattern
            if ((x / 4 + y / 4) % 2 == 0) {
                int byte_pos = (y * ((width + 7) / 8)) + (x / 8);
                int bit_pos = 7 - (x % 8);
                bitmap[byte_pos] |= (1 << bit_pos);
            }
        }
    }
    
    // Print the bitmap
    if (printer.printBitmap(bitmap, width, height)) {
        Serial.println("✓ Printed bitmap pattern");
    } else {
        Serial.println("✗ Failed to print bitmap");
    }
}

void printStatusInfo() {
    Serial.println("\n--- Printer Status ---");
    Serial.printf("Connected: %s\n", printer.isConnected() ? "Yes" : "No");
    
    if (printer.isConnected()) {
        Serial.printf("Name: %s\n", printer.getPrinterName());
        Serial.printf("Max Width: %d px\n", printer.getMaxWidth());
        Serial.printf("Tape Width: %d px\n", printer.getTapeWidth());
        Serial.printf("DPI: %d\n", printer.getDPI());
        
        if (printer.getStatus()) {
            Serial.printf("Media Type: %s\n", printer.getMediaType());
            Serial.printf("Tape Color: %s\n", printer.getTapeColor());
            Serial.printf("Text Color: %s\n", printer.getTextColor());
            
            if (printer.hasError()) {
                Serial.printf("Error: %s\n", printer.getErrorDescription().c_str());
            } else {
                Serial.println("Status: Ready");
            }
        }
    }
    Serial.println("----------------------");
} 