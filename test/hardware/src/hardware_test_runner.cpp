#include "hardware_test_runner.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "usb/usb_host.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

static const char* TAG = "HARDWARE_TEST";

// Adapted TestRegistry for ESP32-S3 hardware
class HardwareTestRegistry {
private:
    struct HardwareTestCase {
        const char* name;
        void (*test_func)();
        const char* category;
    };
    
    static constexpr size_t MAX_TESTS = 50;
    HardwareTestCase tests[MAX_TESTS];
    size_t test_count = 0;
    static HardwareTestRegistry* instance_ptr;
    
    HardwareTestRegistry() = default;
    
public:
    static HardwareTestRegistry& instance() {
        if (!instance_ptr) {
            instance_ptr = new HardwareTestRegistry();
        }
        return *instance_ptr;
    }
    
    void add_test(const char* name, void (*test_func)(), const char* category = "hardware") {
        if (test_count < MAX_TESTS) {
            tests[test_count] = {name, test_func, category};
            test_count++;
        }
    }
    
    // Run tests with ESP-IDF logging
    int run_tests(const char* filter = nullptr, bool verbose = false) {
        int passed = 0;
        int failed = 0;
        
        ESP_LOGI(TAG, "=================================");
        ESP_LOGI(TAG, "ESP32-S3 Hardware Test Suite");
        ESP_LOGI(TAG, "=================================");
        ESP_LOGI(TAG, "Configuration:");
        ESP_LOGI(TAG, "  Verbose: %s", verbose ? "Yes" : "No");
        ESP_LOGI(TAG, "  Filter: %s", filter ? filter : "All tests");
        ESP_LOGI(TAG, "  Tests: %zu", test_count);
        ESP_LOGI(TAG, "");
        
        int64_t start_time = esp_timer_get_time();
        
        for (size_t i = 0; i < test_count; i++) {
            // Apply category filter if specified
            if (filter && strcmp(tests[i].category, filter) != 0) {
                continue;
            }
            
            if (verbose) {
                ESP_LOGI(TAG, "Running %s::%s...", tests[i].category, tests[i].name);
            }
            
            try {
                tests[i].test_func();
                passed++;
                if (verbose) {
                    ESP_LOGI(TAG, "  PASS");
                }
                // Brief delay for watchdog and to avoid overwhelming output
                vTaskDelay(pdMS_TO_TICKS(10));
            } catch (const HardwareTestFailure& e) {
                failed++;
                ESP_LOGE(TAG, "  FAIL: %s", e.what());
            } catch (...) {
                failed++;
                ESP_LOGE(TAG, "  ERROR: Unknown exception");
            }
        }
        
        int64_t end_time = esp_timer_get_time();
        int64_t duration_ms = (end_time - start_time) / 1000;
        
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "Results:");
        ESP_LOGI(TAG, "  Passed: %d", passed);
        ESP_LOGI(TAG, "  Failed: %d", failed);
        ESP_LOGI(TAG, "  Total:  %d", passed + failed);
        ESP_LOGI(TAG, "  Time:   %lld ms", duration_ms);
        ESP_LOGI(TAG, "");
        
        if (failed > 0) {
            ESP_LOGE(TAG, "TESTS FAILED!");
            return 1;
        } else {
            ESP_LOGI(TAG, "ALL TESTS PASSED!");
            return 0;
        }
    }
    
    size_t get_test_count() const {
        return test_count;
    }
};

// Static instance initialization
HardwareTestRegistry* HardwareTestRegistry::instance_ptr = nullptr;

// Hardware test failure exception
HardwareTestFailure::HardwareTestFailure(const char* file, int line, const char* condition) {
    snprintf(message, sizeof(message), "Test failure at %s:%d - %s", file, line, condition);
}

const char* HardwareTestFailure::what() const {
    return message;
}

// Hardware test macros implementation helpers
void hardware_assert_eq_int(int expected, int actual, const char* file, int line, const char* expr) {
    if (expected != actual) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: expected %d, got %d", expr, expected, actual);
        throw HardwareTestFailure(file, line, msg);
    }
}

void hardware_assert_eq_ptr(void* expected, void* actual, const char* file, int line, const char* expr) {
    if (expected != actual) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: expected %p, got %p", expr, expected, actual);
        throw HardwareTestFailure(file, line, msg);
    }
}

void hardware_assert_true(bool condition, const char* file, int line, const char* expr) {
    if (!condition) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s is false", expr);
        throw HardwareTestFailure(file, line, msg);
    }
}

void hardware_assert_esp_ok(esp_err_t result, const char* file, int line, const char* expr) {
    if (result != ESP_OK) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s returned %s", expr, esp_err_to_name(result));
        throw HardwareTestFailure(file, line, msg);
    }
}

// Test registration helpers
void register_hardware_test(const char* name, void (*test_func)(), const char* category) {
    HardwareTestRegistry::instance().add_test(name, test_func, category);
}

// Hardware initialization for testing
static void init_hardware_test_environment() {
    ESP_LOGI(TAG, "Initializing hardware test environment...");
    
    // Initialize NVS for WiFi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize GPIO for LED indicators (if available)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),  // Built-in LED on many ESP32-S3 boards
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "Hardware test environment initialized");
}

// Main hardware test entry point
extern "C" void app_main() {
    ESP_LOGI(TAG, "ESP32-S3 P-touch Hardware Test Suite Starting...");
    ESP_LOGI(TAG, "Free memory: %lu bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    
    // Initialize hardware
    init_hardware_test_environment();
    
    // Flash LED to indicate test start
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(GPIO_NUM_2, 0);
    
    // Run hardware tests
    auto& registry = HardwareTestRegistry::instance();
    
    ESP_LOGI(TAG, "Found %zu hardware tests", registry.get_test_count());
    
    if (registry.get_test_count() == 0) {
        ESP_LOGE(TAG, "No hardware tests found!");
        return;
    }
    
    // Configure test execution based on compile-time flags
#ifdef TEST_VERBOSE
    bool verbose = true;
#else
    bool verbose = false;
#endif

    // Run all tests by default, or filter by category
    const char* filter = nullptr;
    
#ifdef ENABLE_USB_HOST_TESTS
    // Run USB Host tests first
    ESP_LOGI(TAG, "Running USB Host tests...");
    registry.run_tests("usb_host", verbose);
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif

#ifdef ENABLE_WIFI_TESTS
    // Run WiFi tests
    ESP_LOGI(TAG, "Running WiFi tests...");
    registry.run_tests("wifi", verbose);
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif

    // Run all hardware tests
    ESP_LOGI(TAG, "Running all hardware tests...");
    int result = registry.run_tests(filter, verbose);
    
    // Flash LED to indicate test completion
    for (int i = 0; i < 3; i++) {
        gpio_set_level(GPIO_NUM_2, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(GPIO_NUM_2, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (result == 0) {
        ESP_LOGI(TAG, "Hardware test suite completed successfully");
    } else {
        ESP_LOGE(TAG, "Hardware test suite failed");
    }
    
    ESP_LOGI(TAG, "Final free memory: %lu bytes", esp_get_free_heap_size());
    
    // Keep running and blink LED periodically to show we're alive
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        gpio_set_level(GPIO_NUM_2, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(GPIO_NUM_2, 0);
    }
} 