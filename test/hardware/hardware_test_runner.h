#ifndef HARDWARE_TEST_RUNNER_H
#define HARDWARE_TEST_RUNNER_H

#include <cstring>
#include <cstdio>
#include "esp_err.h"

// Hardware test failure exception (adapted for embedded)
class HardwareTestFailure {
private:
    char message[512];
    
public:
    HardwareTestFailure(const char* file, int line, const char* condition);
    const char* what() const;
};

// Function declarations for assertion helpers
void hardware_assert_eq_int(int expected, int actual, const char* file, int line, const char* expr);
void hardware_assert_eq_ptr(void* expected, void* actual, const char* file, int line, const char* expr);
void hardware_assert_true(bool condition, const char* file, int line, const char* expr);
void hardware_assert_esp_ok(esp_err_t result, const char* file, int line, const char* expr);
void register_hardware_test(const char* name, void (*test_func)(), const char* category);

// Hardware test macros - adapted from existing test framework
#define HARDWARE_TEST(name) \
    void hardware_test_##name(); \
    namespace { \
        struct HardwareTestRegistrar_##name { \
            HardwareTestRegistrar_##name() { \
                register_hardware_test(#name, hardware_test_##name, "hardware"); \
            } \
        }; \
        static HardwareTestRegistrar_##name hw_registrar_##name; \
    } \
    void hardware_test_##name()

#define USB_HOST_TEST(name) \
    void usb_host_test_##name(); \
    namespace { \
        struct USBHostTestRegistrar_##name { \
            USBHostTestRegistrar_##name() { \
                register_hardware_test(#name, usb_host_test_##name, "usb_host"); \
            } \
        }; \
        static USBHostTestRegistrar_##name usb_registrar_##name; \
    } \
    void usb_host_test_##name()

#define WIFI_TEST(name) \
    void wifi_test_##name(); \
    namespace { \
        struct WiFiTestRegistrar_##name { \
            WiFiTestRegistrar_##name() { \
                register_hardware_test(#name, wifi_test_##name, "wifi"); \
            } \
        }; \
        static WiFiTestRegistrar_##name wifi_registrar_##name; \
    } \
    void wifi_test_##name()

#define INTEGRATION_HARDWARE_TEST(name) \
    void integration_hw_test_##name(); \
    namespace { \
        struct IntegrationHWTestRegistrar_##name { \
            IntegrationHWTestRegistrar_##name() { \
                register_hardware_test(#name, integration_hw_test_##name, "integration"); \
            } \
        }; \
        static IntegrationHWTestRegistrar_##name integration_hw_registrar_##name; \
    } \
    void integration_hw_test_##name()

// Hardware assertion macros - ESP32-S3 optimized
#define HW_ASSERT_EQ(expected, actual) \
    hardware_assert_eq_int((expected), (actual), __FILE__, __LINE__, #expected " == " #actual)

#define HW_ASSERT_NE(not_expected, actual) \
    do { \
        auto not_exp_val = (not_expected); \
        auto act_val = (actual); \
        if (not_exp_val == act_val) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), #not_expected " (%d) == " #actual " (%d)", not_exp_val, act_val); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

#define HW_ASSERT_TRUE(condition) \
    hardware_assert_true((condition), __FILE__, __LINE__, #condition)

#define HW_ASSERT_FALSE(condition) \
    do { \
        if ((condition)) { \
            throw HardwareTestFailure(__FILE__, __LINE__, #condition " is true"); \
        } \
    } while(0)

#define HW_ASSERT_LT(a, b) \
    do { \
        auto val_a = (a); \
        auto val_b = (b); \
        if (!(val_a < val_b)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), #a " (%lld) >= " #b " (%lld)", (long long)val_a, (long long)val_b); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

#define HW_ASSERT_LE(a, b) \
    do { \
        auto val_a = (a); \
        auto val_b = (b); \
        if (!(val_a <= val_b)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), #a " (%lld) > " #b " (%lld)", (long long)val_a, (long long)val_b); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

#define HW_ASSERT_GT(a, b) \
    do { \
        auto val_a = (a); \
        auto val_b = (b); \
        if (!(val_a > val_b)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), #a " (%lld) <= " #b " (%lld)", (long long)val_a, (long long)val_b); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

#define HW_ASSERT_GE(a, b) \
    do { \
        auto val_a = (a); \
        auto val_b = (b); \
        if (!(val_a >= val_b)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), #a " (%lld) < " #b " (%lld)", (long long)val_a, (long long)val_b); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

#define HW_ASSERT_GTE(a, b) HW_ASSERT_GE(a, b)

#define HW_ASSERT_NULL(pointer) \
    hardware_assert_eq_ptr(nullptr, (void*)(pointer), __FILE__, __LINE__, #pointer " is null")

#define HW_ASSERT_NOT_NULL(pointer) \
    do { \
        if ((pointer) == nullptr) { \
            throw HardwareTestFailure(__FILE__, __LINE__, #pointer " is null"); \
        } \
    } while(0)

// ESP-IDF specific assertions
#define HW_ASSERT_ESP_OK(result) \
    hardware_assert_esp_ok((result), __FILE__, __LINE__, #result)

#define HW_ASSERT_ESP_ERR(expected_err, result) \
    do { \
        esp_err_t exp_err = (expected_err); \
        esp_err_t act_err = (result); \
        if (exp_err != act_err) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "Expected %s, got %s", esp_err_to_name(exp_err), esp_err_to_name(act_err)); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

// Memory and timing assertions for hardware testing
#define HW_ASSERT_MEMORY_AVAILABLE(min_bytes) \
    do { \
        size_t free_mem = esp_get_free_heap_size(); \
        if (free_mem < (min_bytes)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "Insufficient memory: %zu < %zu", free_mem, (size_t)(min_bytes)); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

#define HW_ASSERT_TIMING(statement, max_ms) \
    do { \
        int64_t start_time = esp_timer_get_time(); \
        statement; \
        int64_t end_time = esp_timer_get_time(); \
        int64_t duration_ms = (end_time - start_time) / 1000; \
        if (duration_ms > (max_ms)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "Operation took %lld ms, expected < %d ms", duration_ms, (int)(max_ms)); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

// GPIO testing macros
#define HW_ASSERT_GPIO_LEVEL(gpio_num, expected_level) \
    do { \
        int actual_level = gpio_get_level((gpio_num_t)(gpio_num)); \
        if (actual_level != (expected_level)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "GPIO%d: expected %d, got %d", (gpio_num), (expected_level), actual_level); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

// USB Host specific assertions
#define HW_ASSERT_USB_DEVICE_CONNECTED(device_handle) \
    HW_ASSERT_NOT_NULL(device_handle)

#define HW_ASSERT_USB_TRANSFER_SUCCESS(transfer) \
    do { \
        if ((transfer)->status != USB_TRANSFER_STATUS_COMPLETED) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "USB transfer failed with status %d", (transfer)->status); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

// WiFi specific assertions
#define HW_ASSERT_WIFI_CONNECTED() \
    do { \
        wifi_ap_record_t ap_info; \
        esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info); \
        if (ret != ESP_OK) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "WiFi not connected: %s", esp_err_to_name(ret)); \
            throw HardwareTestFailure(__FILE__, __LINE__, msg); \
        } \
    } while(0)

// Utility macros for hardware testing
#define HW_DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))

#define HW_RETRY_UNTIL_SUCCESS(statement, max_attempts, delay_ms) \
    do { \
        bool success = false; \
        for (int attempt = 0; attempt < (max_attempts) && !success; attempt++) { \
            try { \
                statement; \
                success = true; \
            } catch (...) { \
                if (attempt == (max_attempts) - 1) { \
                    throw; \
                } \
                vTaskDelay(pdMS_TO_TICKS(delay_ms)); \
            } \
        } \
    } while(0)

// Test fixture helper for hardware tests
class HardwareTestFixture {
public:
    virtual ~HardwareTestFixture() = default;
    virtual void setup() {}
    virtual void teardown() {}
    
protected:
    // Common hardware test utilities
    void reset_hardware_state() {
        // Reset GPIO states, clear any pending operations, etc.
    }
    
    void check_memory_leaks(size_t initial_free_mem) {
        size_t current_free_mem = esp_get_free_heap_size();
        if (current_free_mem < initial_free_mem - 1024) { // Allow 1KB tolerance
            char msg[256];
            snprintf(msg, sizeof(msg), "Memory leak detected: %zu -> %zu bytes", 
                    initial_free_mem, current_free_mem);
            throw HardwareTestFailure(__FILE__, __LINE__, msg);
        }
    }
};

#define HARDWARE_TEST_F(fixture_class, test_name) \
    void fixture_hw_test_##test_name(); \
    namespace { \
        struct FixtureHWTestRegistrar_##test_name { \
            FixtureHWTestRegistrar_##test_name() { \
                register_hardware_test(#test_name, []() { \
                    fixture_class fixture; \
                    size_t initial_mem = esp_get_free_heap_size(); \
                    fixture.setup(); \
                    try { \
                        fixture_hw_test_##test_name(); \
                    } catch (...) { \
                        fixture.teardown(); \
                        throw; \
                    } \
                    fixture.teardown(); \
                    fixture.check_memory_leaks(initial_mem); \
                }, "hardware"); \
            } \
        }; \
        static FixtureHWTestRegistrar_##test_name fixture_hw_registrar_##test_name; \
    } \
    void fixture_hw_test_##test_name()

#endif // HARDWARE_TEST_RUNNER_H 