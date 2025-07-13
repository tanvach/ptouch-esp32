/*
 * P-touch ESP32 Debug System
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

#ifndef PTOUCH_DEBUG_H
#define PTOUCH_DEBUG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_console.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usb/usb_host.h"

#ifdef __cplusplus
extern "C" {
#endif

// Debug configuration
#define PTOUCH_DEBUG_PACKET_BUFFER_SIZE    (8192)
#define PTOUCH_DEBUG_MAX_PACKET_SIZE       (256)
#define PTOUCH_DEBUG_MAX_HISTORY_ENTRIES   (100)

// Debug levels
typedef enum {
    PTOUCH_DEBUG_LEVEL_NONE = 0,
    PTOUCH_DEBUG_LEVEL_ERROR,
    PTOUCH_DEBUG_LEVEL_WARN,
    PTOUCH_DEBUG_LEVEL_INFO,
    PTOUCH_DEBUG_LEVEL_DEBUG,
    PTOUCH_DEBUG_LEVEL_VERBOSE
} ptouch_debug_level_t;

// Packet direction
typedef enum {
    PTOUCH_PACKET_DIR_OUT = 0,
    PTOUCH_PACKET_DIR_IN = 1
} ptouch_packet_dir_t;

// Protocol command types
typedef enum {
    PTOUCH_CMD_UNKNOWN = 0,
    PTOUCH_CMD_INIT,
    PTOUCH_CMD_STATUS_REQUEST,
    PTOUCH_CMD_INFO,
    PTOUCH_CMD_PACKBITS_ENABLE,
    PTOUCH_CMD_RASTER_START,
    PTOUCH_CMD_RASTER_LINE,
    PTOUCH_CMD_PRECUT,
    PTOUCH_CMD_FINALIZE,
    PTOUCH_CMD_D460BT_MAGIC,
    PTOUCH_CMD_D460BT_CHAIN,
    PTOUCH_CMD_PAGE_FLAGS,
    PTOUCH_CMD_FEED_PAPER,
    PTOUCH_CMD_CUT_PAPER
} ptouch_protocol_cmd_t;

// Packet information structure
typedef struct {
    int64_t timestamp;                              // Timestamp in microseconds
    ptouch_packet_dir_t direction;                  // IN or OUT
    uint8_t endpoint;                               // USB endpoint address
    size_t length;                                  // Data length
    uint8_t data[PTOUCH_DEBUG_MAX_PACKET_SIZE];     // Packet data
    ptouch_protocol_cmd_t cmd_type;                 // Identified command type
    char cmd_description[64];                       // Human-readable description
    bool is_error;                                  // Error flag
    uint32_t transfer_status;                       // USB transfer status
} ptouch_packet_info_t;

// Debug statistics
typedef struct {
    uint32_t total_packets;
    uint32_t packets_out;
    uint32_t packets_in;
    uint32_t errors;
    uint32_t timeouts;
    uint32_t protocol_errors;
    uint64_t bytes_sent;
    uint64_t bytes_received;
    int64_t last_packet_time;
    int64_t first_packet_time;
} ptouch_debug_stats_t;

// Debug logger class
typedef struct {
    bool enabled;
    ptouch_debug_level_t level;
    void* packet_history;  // Simplified for now
    ptouch_debug_stats_t stats;
    bool console_enabled;
    bool web_enabled;
    const char* log_tag;
} ptouch_debug_logger_t;

// Global debug logger instance
extern ptouch_debug_logger_t* g_ptouch_debug_logger;

// Logger management functions
esp_err_t ptouch_debug_init(ptouch_debug_level_t level);
esp_err_t ptouch_debug_deinit(void);
esp_err_t ptouch_debug_set_level(ptouch_debug_level_t level);
ptouch_debug_level_t ptouch_debug_get_level(void);

// Packet logging functions
esp_err_t ptouch_debug_log_packet(ptouch_packet_dir_t direction, 
                                  uint8_t endpoint,
                                  const uint8_t* data, 
                                  size_t length,
                                  uint32_t transfer_status);

esp_err_t ptouch_debug_log_usb_transfer(const usb_transfer_t* transfer, 
                                        ptouch_packet_dir_t direction);

// Protocol analysis functions
ptouch_protocol_cmd_t ptouch_debug_identify_command(const uint8_t* data, size_t length);
const char* ptouch_debug_get_command_name(ptouch_protocol_cmd_t cmd);
const char* ptouch_debug_get_command_description(const uint8_t* data, size_t length);

// Statistics functions
ptouch_debug_stats_t ptouch_debug_get_stats(void);
void ptouch_debug_reset_stats(void);
void ptouch_debug_print_stats(void);

// Packet history functions
esp_err_t ptouch_debug_get_packet_history(ptouch_packet_info_t* packets, size_t max_count, size_t* actual_count);
void ptouch_debug_print_packet_history(size_t count);
void ptouch_debug_clear_history(void);

// Console command functions
esp_err_t ptouch_debug_register_console_commands(void);
esp_err_t ptouch_debug_unregister_console_commands(void);

// Utility functions
void ptouch_debug_hex_dump(const char* tag, const uint8_t* data, size_t length);
void ptouch_debug_print_transfer_status(uint32_t status);
const char* ptouch_debug_get_transfer_status_string(uint32_t status);

// Convenience macros
#define PTOUCH_DEBUG_LOG_PACKET_OUT(ep, data, len, status) \
    ptouch_debug_log_packet(PTOUCH_PACKET_DIR_OUT, ep, data, len, status)

#define PTOUCH_DEBUG_LOG_PACKET_IN(ep, data, len, status) \
    ptouch_debug_log_packet(PTOUCH_PACKET_DIR_IN, ep, data, len, status)

#define PTOUCH_DEBUG_ENABLED() \
    (g_ptouch_debug_logger && g_ptouch_debug_logger->enabled)

#define PTOUCH_DEBUG_LEVEL_CHECK(level) \
    (PTOUCH_DEBUG_ENABLED() && g_ptouch_debug_logger->level >= level)

// Conditional logging macros
#define PTOUCH_LOGE(format, ...) \
    do { if (PTOUCH_DEBUG_LEVEL_CHECK(PTOUCH_DEBUG_LEVEL_ERROR)) \
         ESP_LOGE("ptouch-debug", format, ##__VA_ARGS__); } while(0)

#define PTOUCH_LOGW(format, ...) \
    do { if (PTOUCH_DEBUG_LEVEL_CHECK(PTOUCH_DEBUG_LEVEL_WARN)) \
         ESP_LOGW("ptouch-debug", format, ##__VA_ARGS__); } while(0)

#define PTOUCH_LOGI(format, ...) \
    do { if (PTOUCH_DEBUG_LEVEL_CHECK(PTOUCH_DEBUG_LEVEL_INFO)) \
         ESP_LOGI("ptouch-debug", format, ##__VA_ARGS__); } while(0)

#define PTOUCH_LOGD(format, ...) \
    do { if (PTOUCH_DEBUG_LEVEL_CHECK(PTOUCH_DEBUG_LEVEL_DEBUG)) \
         ESP_LOGD("ptouch-debug", format, ##__VA_ARGS__); } while(0)

#define PTOUCH_LOGV(format, ...) \
    do { if (PTOUCH_DEBUG_LEVEL_CHECK(PTOUCH_DEBUG_LEVEL_VERBOSE)) \
         ESP_LOGV("ptouch-debug", format, ##__VA_ARGS__); } while(0)

#ifdef __cplusplus
}
#endif

#endif // PTOUCH_DEBUG_H 