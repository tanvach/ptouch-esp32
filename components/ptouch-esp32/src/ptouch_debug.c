/*
 * P-touch ESP32 Debug System Implementation
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

#include "ptouch_debug.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_console.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <argtable3/argtable3.h>

static const char* TAG = "ptouch-debug";

// Global debug logger instance
ptouch_debug_logger_t* g_ptouch_debug_logger = NULL;

// Console command argument structures
static struct {
    struct arg_str *level;
    struct arg_end *end;
} debug_level_args;

static struct {
    struct arg_int *count;
    struct arg_end *end;
} debug_history_args;

static struct {
    struct arg_end *end;
} debug_stats_args;

// Logger management functions
esp_err_t ptouch_debug_init(ptouch_debug_level_t level) {
    if (g_ptouch_debug_logger) {
        ESP_LOGW(TAG, "Debug logger already initialized");
        return ESP_OK;
    }
    
    // Allocate logger structure
    g_ptouch_debug_logger = heap_caps_malloc(sizeof(ptouch_debug_logger_t), MALLOC_CAP_8BIT);
    if (!g_ptouch_debug_logger) {
        ESP_LOGE(TAG, "Failed to allocate debug logger");
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize logger
    memset(g_ptouch_debug_logger, 0, sizeof(ptouch_debug_logger_t));
    g_ptouch_debug_logger->enabled = true;
    g_ptouch_debug_logger->level = level;
    g_ptouch_debug_logger->log_tag = TAG;
    g_ptouch_debug_logger->console_enabled = true;
    g_ptouch_debug_logger->web_enabled = false;
    
    // Simplified packet history (disabled for now)
    g_ptouch_debug_logger->packet_history = NULL;
    
    // Initialize statistics
    g_ptouch_debug_logger->stats.first_packet_time = esp_timer_get_time();
    
    // Register console commands
    ptouch_debug_register_console_commands();
    
    ESP_LOGI(TAG, "Debug logger initialized (level: %d)", level);
    return ESP_OK;
}

esp_err_t ptouch_debug_deinit(void) {
    if (!g_ptouch_debug_logger) {
        return ESP_OK;
    }
    
    // Unregister console commands
    ptouch_debug_unregister_console_commands();
    
    // Clean up packet history (simplified)
    g_ptouch_debug_logger->packet_history = NULL;
    
    // Free logger structure with proper heap_caps_free
    heap_caps_free(g_ptouch_debug_logger);
    g_ptouch_debug_logger = NULL;
    
    ESP_LOGI(TAG, "Debug logger deinitialized");
    return ESP_OK;
}

esp_err_t ptouch_debug_set_level(ptouch_debug_level_t level) {
    if (!g_ptouch_debug_logger) {
        return ESP_ERR_INVALID_STATE;
    }
    
    g_ptouch_debug_logger->level = level;
    ESP_LOGI(TAG, "Debug level set to %d", level);
    return ESP_OK;
}

ptouch_debug_level_t ptouch_debug_get_level(void) {
    if (!g_ptouch_debug_logger) {
        return PTOUCH_DEBUG_LEVEL_NONE;
    }
    return g_ptouch_debug_logger->level;
}

// Protocol analysis functions
ptouch_protocol_cmd_t ptouch_debug_identify_command(const uint8_t* data, size_t length) {
    if (!data || length == 0) {
        return PTOUCH_CMD_UNKNOWN;
    }
    
    // Check for common command patterns
    if (length >= 2) {
        // ESC commands (0x1B)
        if (data[0] == 0x1B) {
            if (length >= 3 && data[1] == 0x69) {
                switch (data[2]) {
                    case 0x53: return PTOUCH_CMD_STATUS_REQUEST;  // ESC i S
                    case 0x7A: return PTOUCH_CMD_INFO;            // ESC i z
                    case 0x52: return PTOUCH_CMD_RASTER_START;    // ESC i R
                    case 0x61: return PTOUCH_CMD_RASTER_START;    // ESC i a (P700)
                    case 0x4D: return PTOUCH_CMD_PRECUT;          // ESC i M
                    case 0x4B: return PTOUCH_CMD_D460BT_CHAIN;    // ESC i K
                    case 0x64: return PTOUCH_CMD_D460BT_MAGIC;    // ESC i d
                }
            }
            if (data[1] == 0x40) {
                return PTOUCH_CMD_INIT;  // ESC @
            }
        }
        
        // PackBits enable command
        if (data[0] == 0x4D && data[1] == 0x02) {
            return PTOUCH_CMD_PACKBITS_ENABLE;
        }
        
        // Raster line command
        if (data[0] == 0x47) {
            return PTOUCH_CMD_RASTER_LINE;
        }
        
        // Single byte commands
        if (length == 1) {
            switch (data[0]) {
                case 0x1A: return PTOUCH_CMD_FINALIZE;    // Print with feed
                case 0x0C: return PTOUCH_CMD_CUT_PAPER;   // Form feed
                case 0x5A: return PTOUCH_CMD_FEED_PAPER;  // Line feed
            }
        }
    }
    
    // Check for long initialization sequence (100+ zeros + ESC @)
    if (length >= 102) {
        bool is_init = true;
        for (size_t i = 0; i < 100; i++) {
            if (data[i] != 0x00) {
                is_init = false;
                break;
            }
        }
        if (is_init && data[100] == 0x1B && data[101] == 0x40) {
            return PTOUCH_CMD_INIT;
        }
    }
    
    return PTOUCH_CMD_UNKNOWN;
}

const char* ptouch_debug_get_command_name(ptouch_protocol_cmd_t cmd) {
    switch (cmd) {
        case PTOUCH_CMD_INIT:               return "INIT";
        case PTOUCH_CMD_STATUS_REQUEST:     return "STATUS_REQ";
        case PTOUCH_CMD_INFO:               return "INFO";
        case PTOUCH_CMD_PACKBITS_ENABLE:    return "PACKBITS_EN";
        case PTOUCH_CMD_RASTER_START:       return "RASTER_START";
        case PTOUCH_CMD_RASTER_LINE:        return "RASTER_LINE";
        case PTOUCH_CMD_PRECUT:             return "PRECUT";
        case PTOUCH_CMD_FINALIZE:           return "FINALIZE";
        case PTOUCH_CMD_D460BT_MAGIC:       return "D460BT_MAGIC";
        case PTOUCH_CMD_D460BT_CHAIN:       return "D460BT_CHAIN";
        case PTOUCH_CMD_PAGE_FLAGS:         return "PAGE_FLAGS";
        case PTOUCH_CMD_FEED_PAPER:         return "FEED_PAPER";
        case PTOUCH_CMD_CUT_PAPER:          return "CUT_PAPER";
        default:                            return "UNKNOWN";
    }
}

const char* ptouch_debug_get_command_description(const uint8_t* data, size_t length) {
    static char desc[64];
    ptouch_protocol_cmd_t cmd = ptouch_debug_identify_command(data, length);
    
    switch (cmd) {
        case PTOUCH_CMD_INIT:
            if (length >= 102) {
                snprintf(desc, sizeof(desc), "Invalidate + Init (%zu bytes)", length);
            } else {
                snprintf(desc, sizeof(desc), "Init command");
            }
            break;
        case PTOUCH_CMD_STATUS_REQUEST:
            snprintf(desc, sizeof(desc), "Status request");
            break;
        case PTOUCH_CMD_INFO:
            snprintf(desc, sizeof(desc), "Info command (%zu bytes)", length);
            break;
        case PTOUCH_CMD_PACKBITS_ENABLE:
            snprintf(desc, sizeof(desc), "Enable PackBits compression");
            break;
        case PTOUCH_CMD_RASTER_START:
            if (length >= 3 && data[1] == 0x69 && data[2] == 0x61) {
                snprintf(desc, sizeof(desc), "Start raster mode (P700)");
            } else {
                snprintf(desc, sizeof(desc), "Start raster mode");
            }
            break;
        case PTOUCH_CMD_RASTER_LINE:
            snprintf(desc, sizeof(desc), "Raster line (%zu bytes)", length);
            break;
        case PTOUCH_CMD_PRECUT:
            snprintf(desc, sizeof(desc), "Precut command");
            break;
        case PTOUCH_CMD_FINALIZE:
            snprintf(desc, sizeof(desc), "Print and eject");
            break;
        case PTOUCH_CMD_D460BT_MAGIC:
            snprintf(desc, sizeof(desc), "D460BT magic sequence");
            break;
        case PTOUCH_CMD_D460BT_CHAIN:
            snprintf(desc, sizeof(desc), "D460BT chain command");
            break;
        case PTOUCH_CMD_FEED_PAPER:
            snprintf(desc, sizeof(desc), "Feed paper (line feed)");
            break;
        case PTOUCH_CMD_CUT_PAPER:
            snprintf(desc, sizeof(desc), "Cut paper (form feed)");
            break;
        default:
            snprintf(desc, sizeof(desc), "Unknown command (%zu bytes)", length);
            break;
    }
    
    return desc;
}

// Packet logging functions
esp_err_t ptouch_debug_log_packet(ptouch_packet_dir_t direction, 
                                  uint8_t endpoint,
                                  const uint8_t* data, 
                                  size_t length,
                                  uint32_t transfer_status) {
    if (!PTOUCH_DEBUG_ENABLED()) {
        return ESP_OK;
    }
    
    if (!data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Create packet info
    ptouch_packet_info_t packet_info = {0};
    packet_info.timestamp = esp_timer_get_time();
    packet_info.direction = direction;
    packet_info.endpoint = endpoint;
    packet_info.length = length > PTOUCH_DEBUG_MAX_PACKET_SIZE ? PTOUCH_DEBUG_MAX_PACKET_SIZE : length;
    packet_info.transfer_status = transfer_status;
    packet_info.is_error = (transfer_status != 0);
    
    // Copy data
    memcpy(packet_info.data, data, packet_info.length);
    
    // Analyze protocol command
    packet_info.cmd_type = ptouch_debug_identify_command(data, length);
    strncpy(packet_info.cmd_description, ptouch_debug_get_command_description(data, length), 
            sizeof(packet_info.cmd_description) - 1);
    
    // Update statistics
    g_ptouch_debug_logger->stats.total_packets++;
    g_ptouch_debug_logger->stats.last_packet_time = packet_info.timestamp;
    
    if (direction == PTOUCH_PACKET_DIR_OUT) {
        g_ptouch_debug_logger->stats.packets_out++;
        g_ptouch_debug_logger->stats.bytes_sent += length;
    } else {
        g_ptouch_debug_logger->stats.packets_in++;
        g_ptouch_debug_logger->stats.bytes_received += length;
    }
    
    if (packet_info.is_error) {
        g_ptouch_debug_logger->stats.errors++;
    }
    
    // Store in packet history (simplified - disabled for now)
    // TODO: Implement simple circular buffer if needed
    
    // Log packet based on debug level
    const char* dir_str = (direction == PTOUCH_PACKET_DIR_OUT) ? "OUT" : "IN";
    const char* cmd_name = ptouch_debug_get_command_name(packet_info.cmd_type);
    
    if (g_ptouch_debug_logger->level >= PTOUCH_DEBUG_LEVEL_INFO) {
        ESP_LOGI(TAG, "%s EP:0x%02X [%s] %s (%zu bytes)", 
                dir_str, endpoint, cmd_name, packet_info.cmd_description, length);
    }
    
    if (g_ptouch_debug_logger->level >= PTOUCH_DEBUG_LEVEL_DEBUG) {
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, data, length, ESP_LOG_DEBUG);
    }
    
    if (packet_info.is_error) {
        ESP_LOGE(TAG, "Transfer error: %s", ptouch_debug_get_transfer_status_string(transfer_status));
    }
    
    return ESP_OK;
}

esp_err_t ptouch_debug_log_usb_transfer(const usb_transfer_t* transfer, 
                                        ptouch_packet_dir_t direction) {
    if (!transfer) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return ptouch_debug_log_packet(direction, transfer->bEndpointAddress,
                                   transfer->data_buffer, transfer->actual_num_bytes,
                                   transfer->status);
}

// Statistics functions
ptouch_debug_stats_t ptouch_debug_get_stats(void) {
    if (!g_ptouch_debug_logger) {
        ptouch_debug_stats_t empty = {0};
        return empty;
    }
    return g_ptouch_debug_logger->stats;
}

void ptouch_debug_reset_stats(void) {
    if (!g_ptouch_debug_logger) {
        return;
    }
    
    memset(&g_ptouch_debug_logger->stats, 0, sizeof(ptouch_debug_stats_t));
    g_ptouch_debug_logger->stats.first_packet_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Statistics reset");
}

void ptouch_debug_print_stats(void) {
    if (!g_ptouch_debug_logger) {
        printf("Debug logger not initialized\n");
        return;
    }
    
    ptouch_debug_stats_t* stats = &g_ptouch_debug_logger->stats;
    int64_t duration = stats->last_packet_time - stats->first_packet_time;
    
    printf("\n=== P-touch Debug Statistics ===\n");
    printf("Total packets: %lu\n", (unsigned long)stats->total_packets);
    printf("  OUT: %lu packets, %llu bytes\n", (unsigned long)stats->packets_out, (unsigned long long)stats->bytes_sent);
    printf("  IN:  %lu packets, %llu bytes\n", (unsigned long)stats->packets_in, (unsigned long long)stats->bytes_received);
    printf("Errors: %lu\n", (unsigned long)stats->errors);
    printf("Timeouts: %lu\n", (unsigned long)stats->timeouts);
    printf("Protocol errors: %lu\n", (unsigned long)stats->protocol_errors);
    
    if (duration > 0) {
        double duration_sec = duration / 1000000.0;
        printf("Duration: %.2f seconds\n", duration_sec);
        printf("Packet rate: %.2f packets/sec\n", (double)stats->total_packets / duration_sec);
        printf("Throughput: %.2f bytes/sec\n", (double)(stats->bytes_sent + stats->bytes_received) / duration_sec);
    }
    printf("===============================\n\n");
}

// Utility functions
void ptouch_debug_hex_dump(const char* tag, const uint8_t* data, size_t length) {
    ESP_LOG_BUFFER_HEX(tag, data, length);
}

const char* ptouch_debug_get_transfer_status_string(uint32_t status) {
    switch (status) {
        case 0: return "SUCCESS";
        case 1: return "ERROR_CRC";
        case 2: return "ERROR_BITSTUFF";
        case 3: return "ERROR_DATA_TOGGLE";
        case 4: return "ERROR_STALL";
        case 5: return "ERROR_DEVICE_NOT_RESPONDING";
        case 6: return "ERROR_PID_CHECK_FAILURE";
        case 7: return "ERROR_UNEXPECTED_PID";
        case 8: return "ERROR_DATA_OVERRUN";
        case 9: return "ERROR_DATA_UNDERRUN";
        case 10: return "ERROR_BUFFER_OVERRUN";
        case 11: return "ERROR_BUFFER_UNDERRUN";
        case 12: return "ERROR_TIMEOUT";
        case 13: return "ERROR_CANCELLED";
        default: return "UNKNOWN_ERROR";
    }
}

void ptouch_debug_print_transfer_status(uint32_t status) {
    printf("Transfer status: %s (%lu)\n", ptouch_debug_get_transfer_status_string(status), (unsigned long)status);
}

// Console command implementations
static int cmd_debug_level(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&debug_level_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, debug_level_args.end, argv[0]);
        return 1;
    }
    
    if (debug_level_args.level->count > 0) {
        const char* level_str = debug_level_args.level->sval[0];
        ptouch_debug_level_t level = PTOUCH_DEBUG_LEVEL_NONE;
        
        if (strcmp(level_str, "none") == 0) level = PTOUCH_DEBUG_LEVEL_NONE;
        else if (strcmp(level_str, "error") == 0) level = PTOUCH_DEBUG_LEVEL_ERROR;
        else if (strcmp(level_str, "warn") == 0) level = PTOUCH_DEBUG_LEVEL_WARN;
        else if (strcmp(level_str, "info") == 0) level = PTOUCH_DEBUG_LEVEL_INFO;
        else if (strcmp(level_str, "debug") == 0) level = PTOUCH_DEBUG_LEVEL_DEBUG;
        else if (strcmp(level_str, "verbose") == 0) level = PTOUCH_DEBUG_LEVEL_VERBOSE;
        else {
            printf("Invalid level. Use: none, error, warn, info, debug, verbose\n");
            return 1;
        }
        
        ptouch_debug_set_level(level);
        printf("Debug level set to: %s\n", level_str);
    } else {
        ptouch_debug_level_t current = ptouch_debug_get_level();
        const char* level_names[] = {"none", "error", "warn", "info", "debug", "verbose"};
        printf("Current debug level: %s\n", level_names[current]);
    }
    
    return 0;
}

static int cmd_debug_stats(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&debug_stats_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, debug_stats_args.end, argv[0]);
        return 1;
    }
    
    ptouch_debug_print_stats();
    return 0;
}

static int cmd_debug_reset(int argc, char **argv) {
    ptouch_debug_reset_stats();
    ptouch_debug_clear_history();
    printf("Debug statistics and history cleared\n");
    return 0;
}

static int cmd_debug_history(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&debug_history_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, debug_history_args.end, argv[0]);
        return 1;
    }
    
    size_t count = 10;  // Default
    if (debug_history_args.count->count > 0) {
        count = debug_history_args.count->ival[0];
    }
    
    ptouch_debug_print_packet_history(count);
    return 0;
}

// Packet history functions
void ptouch_debug_print_packet_history(size_t count) {
    if (!g_ptouch_debug_logger) {
        printf("Debug logger not initialized\n");
        return;
    }
    
    printf("\n=== Packet History (last %zu packets) ===\n", count);
    printf("No packet history available (feature disabled)\n");
    printf("Use real-time logging or statistics instead:\n");
    printf("  - debug_level verbose  # Enable verbose logging\n");
    printf("  - debug_stats          # Show statistics\n");
    printf("=====================================\n\n");
}

esp_err_t ptouch_debug_get_packet_history(ptouch_packet_info_t* packets, size_t max_count, size_t* actual_count) {
    if (!g_ptouch_debug_logger || !packets || !actual_count) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Simplified - no packet history for now
    *actual_count = 0;
    return ESP_OK;
}

void ptouch_debug_clear_history(void) {
    if (!g_ptouch_debug_logger) {
        return;
    }
    
    // Simplified - no packet history to clear
    // TODO: Implement if simple circular buffer is added
}

// Console command registration
esp_err_t ptouch_debug_register_console_commands(void) {
    // Initialize argument structures
    debug_level_args.level = arg_str0("l", "level", "<level>", "Set debug level (none/error/warn/info/debug/verbose)");
    debug_level_args.end = arg_end(2);
    
    debug_history_args.count = arg_int0("c", "count", "<count>", "Number of packets to show (default: 10)");
    debug_history_args.end = arg_end(2);
    
    debug_stats_args.end = arg_end(2);
    
    // Register commands with proper error handling
    const esp_console_cmd_t debug_level_cmd = {
        .command = "debug_level",
        .help = "Get/set debug logging level",
        .hint = NULL,
        .func = &cmd_debug_level,
        .argtable = &debug_level_args
    };
    
    const esp_console_cmd_t debug_stats_cmd = {
        .command = "debug_stats",
        .help = "Show debug statistics",
        .hint = NULL,
        .func = &cmd_debug_stats,
        .argtable = &debug_stats_args
    };
    
    const esp_console_cmd_t debug_reset_cmd = {
        .command = "debug_reset",
        .help = "Reset debug statistics and history",
        .hint = NULL,
        .func = &cmd_debug_reset,
        .argtable = &debug_stats_args
    };
    
    const esp_console_cmd_t debug_history_cmd = {
        .command = "debug_history",
        .help = "Show packet history",
        .hint = NULL,
        .func = &cmd_debug_history,
        .argtable = &debug_history_args
    };
    
    esp_err_t ret = ESP_OK;
    
    ret = esp_console_cmd_register(&debug_level_cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register debug_level command: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_console_cmd_register(&debug_stats_cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register debug_stats command: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_console_cmd_register(&debug_reset_cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register debug_reset command: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_console_cmd_register(&debug_history_cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register debug_history command: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Debug console commands registered successfully");
    return ESP_OK;
}

esp_err_t ptouch_debug_unregister_console_commands(void) {
    // ESP-IDF doesn't provide unregister function, commands persist until reboot
    ESP_LOGW(TAG, "Console commands cannot be unregistered, they persist until reboot");
    return ESP_OK;
} 