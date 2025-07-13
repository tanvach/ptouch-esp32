# P-touch ESP32 Debug Guide

Complete guide to debugging Brother P-touch printer communication using the built-in USB packet logger and diagnostic tools.

## Table of Contents

1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Debug Logging System](#debug-logging-system)
4. [Interactive Console Commands](#interactive-console-commands)
5. [Protocol Analysis](#protocol-analysis)
6. [Troubleshooting Common Issues](#troubleshooting-common-issues)
7. [Advanced Debugging](#advanced-debugging)
8. [Performance Analysis](#performance-analysis)
9. [Real-World Examples](#real-world-examples)
10. [Current Limitations](#current-limitations)

## Overview

The P-touch ESP32 debug system provides comprehensive USB packet logging, protocol analysis, and interactive debugging capabilities using ESP-IDF's built-in components. This system helps diagnose communication issues with Brother P-touch printers.

### Key Features

- **USB Packet Logging**: Complete capture of all USB transfers with timestamps
- **Protocol Analysis**: Automatic identification of P-touch protocol commands
- **Interactive Console**: Real-time debugging commands via serial interface
- **Performance Metrics**: Throughput, error rates, and timing analysis
- **ESP-IDF Integration**: Leverages ESP_LOG, console, and timer components

### Current Limitations

⚠️ **Important**: The packet history functionality is currently simplified and disabled. The following features are not yet implemented:
- Packet history storage and retrieval
- Ring buffer for packet storage
- Historical packet analysis

All other debug features (logging, statistics, protocol analysis) are fully functional.

## Quick Start

### 1. Enable Debug Logging

```cpp
#include "ptouch_esp32.h"

PtouchPrinter printer;

void setup() {
    // Enable debug logging with INFO level
    printer.enableDebugLogging(PTOUCH_DEBUG_LEVEL_INFO);
    
    // Initialize printer
    if (printer.begin()) {
        ESP_LOGI("main", "Printer initialized with debug logging");
    }
}
```

### 2. Monitor Serial Output

```bash
# Open serial monitor
pio device monitor --filter=esp32_exception_decoder

# You'll see output like:
# I (1234) ptouch-debug: OUT EP:0x02 [INIT] Invalidate + Init (102 bytes)
# I (1235) ptouch-debug: OUT EP:0x02 [PACKBITS_EN] Enable PackBits compression (2 bytes)
# I (1236) ptouch-debug: IN EP:0x81 [STATUS_REQ] Status request (32 bytes)
```

### 3. Interactive Console Commands

```bash
# Access interactive console via serial
debug_level info          # Set debug level
debug_stats              # Show statistics
debug_history -c 20      # Show last 20 packets
debug_reset              # Clear statistics
```

## Debug Logging System

### Debug Levels

The debug system supports multiple logging levels:

| Level | Description | Output |
|-------|-------------|--------|
| `NONE` | No debug output | Silent |
| `ERROR` | Errors only | Transfer failures, protocol errors |
| `WARN` | Warnings + errors | Timeouts, unknown commands |
| `INFO` | General info | Packet summary, command identification |
| `DEBUG` | Detailed logging | Hex dumps, timing info |
| `VERBOSE` | Everything | Full protocol analysis |

### Programmatic Control

```cpp
// Enable debug logging
printer.enableDebugLogging(PTOUCH_DEBUG_LEVEL_DEBUG);

// Change debug level at runtime
printer.setDebugLevel(PTOUCH_DEBUG_LEVEL_VERBOSE);

// Check current level
ptouch_debug_level_t level = printer.getDebugLevel();

// Disable debug logging
printer.disableDebugLogging();
```

### C API Usage

```c
#include "ptouch_debug.h"

// Initialize debug logger
ptouch_debug_init(PTOUCH_DEBUG_LEVEL_INFO);

// Log custom packets
uint8_t data[] = {0x1B, 0x40}; // ESC @
ptouch_debug_log_packet(PTOUCH_PACKET_DIR_OUT, 0x02, data, sizeof(data), 0);

// Get statistics
ptouch_debug_stats_t stats = ptouch_debug_get_stats();
printf("Total packets: %u\n", stats.total_packets);
```

## Interactive Console Commands

### Setup Console

The console is automatically initialized when debug logging is enabled. Access via serial terminal:

```bash
# 115200 baud, 8N1
pio device monitor --baud 115200
```

### Available Commands

#### `debug_level [level]`
Get or set the debug logging level.

```bash
# Get current level
debug_level

# Set to debug level
debug_level debug

# Available levels: none, error, warn, info, debug, verbose
```

#### `debug_stats`
Show comprehensive debugging statistics.

```bash
debug_stats

# Output:
# === P-touch Debug Statistics ===
# Total packets: 245
#   OUT: 123 packets, 2048 bytes
#   IN:  122 packets, 3840 bytes
# Errors: 0
# Duration: 12.34 seconds
# Packet rate: 19.87 packets/sec
# Throughput: 476.12 bytes/sec
# ===============================
```

#### `debug_history [-c count]`
Show packet history with optional count limit.

⚠️ **Note**: Packet history is currently disabled. This command will show "No packet history available" message.

```bash
# Show last 10 packets (currently disabled)
debug_history

# Show last 50 packets (currently disabled)
debug_history -c 50

# Current output:
# === Packet History (last 10 packets) ===
# No packet history available (feature disabled)
# =========================================
```

#### `debug_reset`
Reset statistics and clear packet history.

```bash
debug_reset
# Output: Debug statistics and history cleared
```

## Protocol Analysis

### Automatic Command Identification

The debug system automatically identifies P-touch protocol commands:

| Command | Pattern | Description |
|---------|---------|-------------|
| `INIT` | `0x1B 0x40` or 100×`0x00` + `0x1B 0x40` | Initialize printer |
| `STATUS_REQ` | `0x1B 0x69 0x53` | Request printer status |
| `INFO` | `0x1B 0x69 0x7A` | Print info command |
| `PACKBITS_EN` | `0x4D 0x02` | Enable PackBits compression |
| `RASTER_START` | `0x1B 0x69 0x52` or `0x1B 0x69 0x61` | Start raster mode |
| `RASTER_LINE` | `0x47` | Raster line data |
| `FINALIZE` | `0x1A` | Print and eject |
| `D460BT_MAGIC` | `0x1B 0x69 0x64` | D460BT magic sequence |

### Custom Protocol Analysis

```cpp
// Analyze custom data
uint8_t data[] = {0x1B, 0x69, 0x53};
ptouch_protocol_cmd_t cmd = ptouch_debug_identify_command(data, sizeof(data));
const char* name = ptouch_debug_get_command_name(cmd);
// name = "STATUS_REQ"
```

## Troubleshooting Common Issues

### 1. Printer Not Detected

**Symptoms:**
- No USB device enumeration
- "No supported printer found" message

**Debug Steps:**
```bash
# Enable verbose logging
debug_level verbose

# Check USB host initialization
# Look for: "USB Host initialized successfully"

# Check device enumeration
# Look for: "Found USB device: VID=0x04F9, PID=0x..."
```

**Common Solutions:**
- Verify USB cable connection
- Check 5V power supply (2A minimum)
- Try different USB cable
- Ensure printer is powered on
- Check for P-Lite mode (switch to E position)

### 2. Communication Timeouts

**Symptoms:**
- Transfer timeout errors
- Incomplete packet transmission

**Debug Steps:**
```bash
# Check for timeout errors
debug_stats
# Look for: "Timeouts: X" where X > 0

# Examine packet history
debug_history -c 50
# Look for: "ERROR: ERROR_TIMEOUT"
```

**Common Solutions:**
- Increase timeout values in configuration
- Check USB Host power supply
- Verify printer is not in sleep mode
- Reduce data transfer size

### 3. Protocol Errors

**Symptoms:**
- Print jobs fail
- Printer responds with errors
- Incorrect raster data

**Debug Steps:**
```bash
# Enable debug level to see hex dumps
debug_level debug

# Look for protocol command sequence
debug_history -c 100

# Check for missing commands:
# 1. INIT (should be first)
# 2. STATUS_REQ (get printer status)
# 3. PACKBITS_EN (if supported)
# 4. INFO (for newer printers)
# 5. RASTER_START
# 6. RASTER_LINE (multiple)
# 7. FINALIZE
```

**Common Solutions:**
- Verify initialization sequence
- Check PackBits compression support
- Validate raster data format
- Ensure proper command ordering

### 4. Memory Issues

**Symptoms:**
- Debug logger fails to initialize
- Packet history missing entries

**Debug Steps:**
```bash
# Check memory usage
debug_stats
# Monitor heap usage over time

# Reduce debug buffer size if needed
# Edit: PTOUCH_DEBUG_PACKET_BUFFER_SIZE in ptouch_debug.h
```

**Common Solutions:**
- Reduce debug buffer size
- Lower debug level
- Free unused memory
- Increase main task stack size

### 5. Performance Issues

**Symptoms:**
- Slow printing
- High latency
- Buffer overruns

**Debug Steps:**
```bash
# Check performance metrics
debug_stats
# Look for: "Packet rate", "Throughput"

# Analyze timing
debug_level verbose
# Look for timing between commands
```

**Common Solutions:**
- Optimize packet size
- Reduce debug logging level
- Check for busy-waiting loops
- Verify adequate CPU resources

## Advanced Debugging

### Custom Packet Injection

```cpp
// Inject custom commands for testing
uint8_t test_cmd[] = {0x1B, 0x69, 0x53}; // Status request
if (printer.usbSend(test_cmd, sizeof(test_cmd)) > 0) {
    ESP_LOGI("test", "Command sent successfully");
}
```

### Real-time Monitoring

```cpp
// Real-time packet monitoring
void packet_monitor_task(void *arg) {
    while (1) {
        ptouch_debug_stats_t stats = ptouch_debug_get_stats();
        ESP_LOGI("monitor", "Packets: %u, Errors: %u", 
                stats.total_packets, stats.errors);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

### Memory-Efficient Logging

```cpp
// Reduce memory usage for production
#define PTOUCH_DEBUG_PACKET_BUFFER_SIZE    (2048)  // Reduce buffer
#define PTOUCH_DEBUG_MAX_HISTORY_ENTRIES   (50)    // Fewer entries

// Use lower debug level
printer.setDebugLevel(PTOUCH_DEBUG_LEVEL_WARN);
```

## Performance Analysis

### Throughput Analysis

```bash
# Get detailed performance metrics
debug_stats

# Example output analysis:
# Duration: 12.34 seconds        → Total time
# Packet rate: 19.87 packets/sec → Communication frequency
# Throughput: 476.12 bytes/sec   → Data transfer rate
```

**Performance Benchmarks:**
- Normal printing: 15-25 packets/sec
- Raster data: 200-500 bytes/sec
- Status queries: <1 packet/sec

### Error Rate Analysis

```bash
# Check error patterns
debug_history -c 100 | grep ERROR

# Common error patterns:
# - Frequent timeouts → Power/cable issues
# - CRC errors → Data corruption
# - Stall conditions → Protocol violations
```

### Timing Analysis

```cpp
// Add timing measurements
int64_t start_time = esp_timer_get_time();
bool result = printer.printText("Hello World");
int64_t end_time = esp_timer_get_time();
int64_t duration = end_time - start_time;
ESP_LOGI("timing", "Print job took %lld microseconds", duration);
```

## Real-World Examples

### Example 1: Diagnosing Connection Issues

**Problem:** Printer connects but doesn't print

**Debug Session:**
```bash
# Enable verbose logging
debug_level verbose

# Check connection sequence
debug_history -c 20

# Expected sequence:
# 1. INIT command
# 2. STATUS_REQ → should return 32 bytes
# 3. Printer status should show no errors

# If status request fails → connection issue
# If status shows errors → printer problem
```

### Example 2: Optimizing Print Performance

**Problem:** Slow printing performance

**Debug Session:**
```bash
# Monitor performance
debug_stats

# If packet rate < 10/sec → bottleneck in code
# If throughput < 100 bytes/sec → USB issue
# If errors > 5% → reliability problem

# Optimize by:
# 1. Reducing debug level
# 2. Batching small transfers
# 3. Using DMA transfers
```

### Example 3: Protocol Compliance Testing

**Problem:** Printer doesn't recognize commands

**Debug Session:**
```bash
# Enable debug hex dumps
debug_level debug

# Compare with original ptouch-print:
# 1. INIT: Must be exactly 102 bytes (100 zeros + ESC @)
# 2. PACKBITS: Must be 0x4D 0x02 (not 0x4D 0x00)
# 3. INFO: Correct byte positioning for media width
# 4. RASTER: Proper compression format
```

### Example 4: Memory Leak Detection

**Problem:** System crashes after extended use

**Debug Session:**
```cpp
// Monitor memory usage
void memory_monitor(void) {
    size_t free_heap = esp_get_free_heap_size();
    size_t min_heap = esp_get_minimum_free_heap_size();
    
    ESP_LOGI("memory", "Free heap: %u, Min heap: %u", free_heap, min_heap);
    
    // Check debug buffer usage
    ptouch_debug_stats_t stats = ptouch_debug_get_stats();
    ESP_LOGI("memory", "Debug packets: %u", stats.total_packets);
}
```

## Current Limitations

### Packet History Functionality

The packet history feature is currently simplified and disabled to reduce memory usage and complexity. The following limitations exist:

1. **No Packet Storage**: Packets are not stored in a ring buffer
2. **History Commands**: `debug_history` command returns empty results
3. **Memory Efficiency**: This reduces memory usage by ~8KB
4. **Statistics Only**: Only aggregate statistics are maintained

### Memory Usage

Current memory usage with debug system:
- **Debug Logger**: ~200 bytes (struct + statistics)
- **Console Commands**: ~1KB (argument tables)
- **No Packet Buffer**: 0 bytes (disabled)
- **Total Overhead**: ~1.2KB instead of ~8KB

### Performance Impact

- **Logging**: Minimal impact when debug level is INFO or below
- **Statistics**: Near-zero overhead for counters
- **Protocol Analysis**: ~10 microseconds per packet
- **Console**: No impact unless actively used

## Enabling Packet History (Future Implementation)

To enable packet history in the future, the following changes would be needed:

```c
// In ptouch_debug.c - Example implementation
typedef struct {
    ptouch_packet_info_t packets[PTOUCH_DEBUG_MAX_HISTORY_ENTRIES];
    size_t write_index;
    size_t count;
} ptouch_packet_history_t;

// Initialize packet history
ptouch_packet_history_t* history = heap_caps_malloc(sizeof(ptouch_packet_history_t), MALLOC_CAP_8BIT);
g_ptouch_debug_logger->packet_history = history;
```

## Workarounds for Missing Packet History

### 1. Real-time Monitoring

```cpp
// Monitor packets in real-time instead of using history
void monitor_packets() {
    // Enable verbose logging to see all packets
    printer.setDebugLevel(PTOUCH_DEBUG_LEVEL_VERBOSE);
    
    // All packets will be logged immediately
    printer.printText("Test");
}
```

### 2. Statistics-Based Analysis

```cpp
// Use statistics instead of packet history
void analyze_performance() {
    ptouch_debug_reset_stats();
    
    // Perform operation
    printer.printText("Test");
    
    // Check statistics
    ptouch_debug_stats_t stats = ptouch_debug_get_stats();
    printf("Packets: %u, Errors: %u, Throughput: %.2f bytes/sec\n", 
           stats.total_packets, stats.errors, 
           (double)(stats.bytes_sent + stats.bytes_received) / 
           ((stats.last_packet_time - stats.first_packet_time) / 1000000.0));
}
```

### 3. Custom Packet Logging

```cpp
// Implement custom packet logging if needed
static void custom_packet_handler(const uint8_t* data, size_t len, const char* direction) {
    printf("%s: ", direction);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

// Use in your code
custom_packet_handler(data, len, "OUT");
```

## Integration with Existing Code

### Adding Debug to Existing Projects

```cpp
// In your existing main.cpp
#include "ptouch_esp32.h"

PtouchPrinter printer;

void app_main() {
    // Add debug initialization
    printer.enableDebugLogging(PTOUCH_DEBUG_LEVEL_INFO);
    
    // Your existing code
    if (printer.begin()) {
        if (printer.detectPrinter()) {
            if (printer.connect()) {
                // Debug will automatically log all USB transfers
                printer.printText("Debug test");
            }
        }
    }
}
```

### Conditional Debug Compilation

```cpp
// In your project configuration
#ifdef CONFIG_DEBUG_ENABLED
    printer.enableDebugLogging(PTOUCH_DEBUG_LEVEL_DEBUG);
#else
    printer.enableDebugLogging(PTOUCH_DEBUG_LEVEL_ERROR);
#endif
```

### Web-based Debug Interface

```cpp
// Add debug endpoints to existing web server
httpd_uri_t debug_stats_uri = {
    .uri = "/debug/stats",
    .method = HTTP_GET,
    .handler = [](httpd_req_t *req) -> esp_err_t {
        ptouch_debug_stats_t stats = ptouch_debug_get_stats();
        char response[512];
        snprintf(response, sizeof(response), 
                "{\"packets\":%u,\"errors\":%u,\"bytes_sent\":%llu,\"history_disabled\":true}",
                stats.total_packets, stats.errors, stats.bytes_sent);
        httpd_resp_send(req, response, strlen(response));
        return ESP_OK;
    }
};
```

## Best Practices

### 1. **Production Debugging**
- Use `INFO` level for production to minimize overhead
- Monitor error rates continuously using statistics
- Implement automatic debug level escalation on errors

### 2. **Development Debugging**
- Use `DEBUG` or `VERBOSE` levels during development
- Use real-time logging instead of packet history
- Test with multiple printer models

### 3. **Performance Optimization**
- Disable debug logging for timing-critical sections
- Use lower debug levels for high-frequency operations
- Current implementation has minimal memory footprint

### 4. **Error Handling**
- Always check debug logger initialization return values
- Implement fallback behavior if debug logging fails
- Use debug statistics for automated health checks

---

**Need Help?**
- Check the [main README](../README.md) for setup instructions
- Report issues with debug logs attached
- Use statistics and real-time logging for troubleshooting
- Note: Packet history feature is currently disabled

The debug system provides powerful tools for diagnosing and resolving P-touch printer communication issues. While packet history is currently disabled, the real-time logging and statistics provide comprehensive debugging capabilities for most use cases. 