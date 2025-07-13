/*
 * P-touch ESP32 Utility Functions
 * Copyright (C) 2024 tanvach
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Based on the ptouch-print library:
 * https://git.familie-radermacher.ch/linux/ptouch-print.git
 * Copyright (C) Familie Radermacher and contributors
 * Licensed under GPL-3.0
 */

#include "ptouch_esp32.h"

// Media type strings (ported from original library)
const char* pt_mediatype_string(uint8_t media_type) {
    switch (media_type) {
        case 0x00: return "No media";
        case 0x01: return "Laminated tape";
        case 0x03: return "Non-laminated tape";
        case 0x04: return "Fabric tape";
        case 0x11: return "Heat-shrink tube";
        case 0x13: return "Fle tape";
        case 0x14: return "Flexible ID tape";
        case 0x15: return "Satin tape";
        case 0xff: return "Incompatible tape";
        default: return "unknown";
    }
}

// Tape color strings (ported from original library)
const char* pt_tapecolor_string(uint8_t tape_color) {
    switch (tape_color) {
        case 0x01: return "White";
        case 0x02: return "Other";
        case 0x03: return "Clear";
        case 0x04: return "Red";
        case 0x05: return "Blue";
        case 0x06: return "Yellow";
        case 0x07: return "Green";
        case 0x08: return "Black";
        case 0x09: return "Clear";
        case 0x20: return "Matte White";
        case 0x21: return "Matte Clear";
        case 0x22: return "Matte Silver";
        case 0x23: return "Satin Gold";
        case 0x24: return "Satin Silver";
        case 0x30: return "Blue (TZe-5[345]5)";
        case 0x31: return "Red (TZe-435)";
        case 0x40: return "Fluorescent Orange";
        case 0x41: return "Fluorescent Yellow";
        case 0x50: return "Berry Pink (TZe-MQP35)";
        case 0x51: return "Light Gray (TZe-MQL35)";
        case 0x52: return "Lime Green (TZe-MQG35)";
        case 0x60: return "Yellow";
        case 0x61: return "Pink";
        case 0x62: return "Blue";
        case 0x70: return "Heat-shrink Tube";
        case 0x90: return "White(Flex. ID)";
        case 0x91: return "Yellow(Flex. ID)";
        case 0xf0: return "Cleaning";
        case 0xf1: return "Stencil";
        case 0xff: return "Incompatible";
        default: return "unknown";
    }
}

// Text color strings (ported from original library)
const char* pt_textcolor_string(uint8_t text_color) {
    switch (text_color) {
        case 0x01: return "White";
        case 0x02: return "Other";
        case 0x04: return "Red";
        case 0x05: return "Blue";
        case 0x08: return "Black";
        case 0x0a: return "Gold";
        case 0x62: return "Blue(F)";
        case 0xf0: return "Cleaning";
        case 0xf1: return "Stencil";
        case 0xff: return "Incompatible";
        default: return "unknown";
    }
}

// Utility functions for the printer class
const char* PtouchPrinter::getMediaType() const {
    return pt_mediatype_string(status ? status->media_type : 0);
}

const char* PtouchPrinter::getTapeColor() const {
    return pt_tapecolor_string(status ? status->tape_color : 0);
}

const char* PtouchPrinter::getTextColor() const {
    return pt_textcolor_string(status ? status->text_color : 0);
} 