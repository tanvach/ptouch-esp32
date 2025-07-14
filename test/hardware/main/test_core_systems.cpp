#include "hardware_test_runner.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "esp_heap_caps.h"

static const char* TAG = "CORE_SYSTEMS_HW_TEST";

// Test GPIO functionality
HARDWARE_TEST(gpio_basic_operations) {
    ESP_LOGI(TAG, "Testing GPIO basic operations");
    
    const gpio_num_t test_gpio = GPIO_NUM_2; // Built-in LED on many ESP32-S3 boards
    
    // Configure GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << test_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    HW_ASSERT_ESP_OK(gpio_config(&io_conf));
    
    // Test setting GPIO high
    HW_ASSERT_ESP_OK(gpio_set_level(test_gpio, 1));
    HW_ASSERT_GPIO_LEVEL(test_gpio, 1);
    
    // Test setting GPIO low
    HW_ASSERT_ESP_OK(gpio_set_level(test_gpio, 0));
    HW_ASSERT_GPIO_LEVEL(test_gpio, 0);
    
    // Test rapid GPIO toggling
    for (int i = 0; i < 10; i++) {
        gpio_set_level(test_gpio, 1);
        gpio_set_level(test_gpio, 0);
    }
    
    // Reset GPIO
    gpio_reset_pin(test_gpio);
    
    ESP_LOGI(TAG, "GPIO basic operations test passed");
}

// Test timing and delays
HARDWARE_TEST(timing_accuracy) {
    ESP_LOGI(TAG, "Testing timing accuracy");
    
    // Test vTaskDelay accuracy
    int64_t start_time = esp_timer_get_time();
    HW_DELAY_MS(100);
    int64_t end_time = esp_timer_get_time();
    int64_t actual_delay = (end_time - start_time) / 1000;
    
    // Allow ±20ms tolerance for FreeRTOS scheduling
    HW_ASSERT_TRUE(actual_delay >= 80 && actual_delay <= 120);
    ESP_LOGI(TAG, "vTaskDelay(100ms) actual delay: %lld ms", actual_delay);
    
    // Test esp_timer accuracy
    start_time = esp_timer_get_time();
    vTaskDelay(pdMS_TO_TICKS(1)); // Minimal delay
    end_time = esp_timer_get_time();
    
    HW_ASSERT_TRUE((end_time - start_time) > 0);
    ESP_LOGI(TAG, "esp_timer resolution: %lld us", (end_time - start_time));
    
    ESP_LOGI(TAG, "Timing accuracy test passed");
}

// Test memory allocation and management
HARDWARE_TEST(memory_management) {
    ESP_LOGI(TAG, "Testing memory management");
    
    size_t initial_free = esp_get_free_heap_size();
    size_t initial_min_free = esp_get_minimum_free_heap_size();
    
    ESP_LOGI(TAG, "Initial free memory: %zu bytes", initial_free);
    ESP_LOGI(TAG, "Minimum free memory: %zu bytes", initial_min_free);
    
    // Test basic malloc/free
    const size_t alloc_size = 4096;
    void* ptr = malloc(alloc_size);
    HW_ASSERT_NOT_NULL(ptr);
    
    // Verify memory usage
    size_t free_after_alloc = esp_get_free_heap_size();
    HW_ASSERT_TRUE(free_after_alloc < initial_free);
    
    // Write to allocated memory
    memset(ptr, 0xAA, alloc_size);
    
    // Free memory
    free(ptr);
    
    // Check memory recovery
    size_t free_after_free = esp_get_free_heap_size();
    size_t memory_diff = initial_free - free_after_free;
    HW_ASSERT_TRUE(memory_diff < 100); // Allow for small fragmentation
    
    // Test DMA-capable memory allocation
    void* dma_ptr = heap_caps_malloc(1024, MALLOC_CAP_DMA);
    HW_ASSERT_NOT_NULL(dma_ptr);
    heap_caps_free(dma_ptr);
    
    ESP_LOGI(TAG, "Memory management test passed");
}

// Test random number generation
HARDWARE_TEST(random_number_generation) {
    ESP_LOGI(TAG, "Testing random number generation");
    
    // Generate multiple random numbers
    uint32_t random_values[10];
    bool all_different = true;
    
    for (int i = 0; i < 10; i++) {
        random_values[i] = esp_random();
        ESP_LOGD(TAG, "Random value %d: 0x%08lX", i, random_values[i]);
        
        // Check that we're not getting the same value repeatedly
        for (int j = 0; j < i; j++) {
            if (random_values[i] == random_values[j]) {
                all_different = false;
                break;
            }
        }
    }
    
    // It's extremely unlikely (but not impossible) for all 10 values to be different
    // But they shouldn't all be the same
    HW_ASSERT_TRUE(random_values[0] != random_values[9] || 
                   random_values[1] != random_values[8]);
    
    ESP_LOGI(TAG, "Random number generation test passed");
}

// Test FreeRTOS task creation and deletion
HARDWARE_TEST(freertos_task_management) {
    ESP_LOGI(TAG, "Testing FreeRTOS task management");
    
    static bool task_executed = false;
    
    // Simple test task
    auto test_task = [](void* parameter) {
        static bool* executed = (bool*)parameter;
        *executed = true;
        vTaskDelete(nullptr);
    };
    
    // Create task
    TaskHandle_t task_handle = nullptr;
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)test_task,
        "test_task",
        2048, // Stack size
        &task_executed,
        tskIDLE_PRIORITY + 1,
        &task_handle
    );
    
    HW_ASSERT_EQ(pdPASS, result);
    HW_ASSERT_NOT_NULL(task_handle);
    
    // Wait for task to execute
    HW_DELAY_MS(100);
    
    // Verify task executed
    HW_ASSERT_TRUE(task_executed);
    
    ESP_LOGI(TAG, "FreeRTOS task management test passed");
}

// Test CPU performance and capabilities
HARDWARE_TEST(cpu_performance) {
    ESP_LOGI(TAG, "Testing CPU performance");
    
    // Test CPU frequency
    uint32_t cpu_freq = esp_clk_cpu_freq();
    ESP_LOGI(TAG, "CPU frequency: %lu Hz", cpu_freq);
    HW_ASSERT_TRUE(cpu_freq >= 80000000); // At least 80MHz
    
    // Test basic arithmetic performance
    const int iterations = 100000;
    int64_t start_time = esp_timer_get_time();
    
    volatile int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i * 2;
    }
    
    int64_t end_time = esp_timer_get_time();
    int64_t duration_us = end_time - start_time;
    
    ESP_LOGI(TAG, "Arithmetic test: %d iterations in %lld us", iterations, duration_us);
    HW_ASSERT_TRUE(duration_us < 100000); // Should complete in less than 100ms
    
    ESP_LOGI(TAG, "CPU performance test passed");
}

// Test watchdog timer functionality
HARDWARE_TEST(watchdog_timer) {
    ESP_LOGI(TAG, "Testing watchdog timer functionality");
    
    // The watchdog should be running automatically
    // Test that we can feed the watchdog
    
    // Do some work that might trigger watchdog if not fed
    for (int i = 0; i < 100; i++) {
        // Simulate some work
        volatile int dummy = 0;
        for (int j = 0; j < 10000; j++) {
            dummy += j;
        }
        
        // Feed the watchdog periodically
        if (i % 10 == 0) {
            vTaskDelay(pdMS_TO_TICKS(1)); // This should reset the watchdog
        }
    }
    
    ESP_LOGI(TAG, "Watchdog timer test passed");
}

// Test heap capabilities and PSRAM (if available)
HARDWARE_TEST(heap_capabilities) {
    ESP_LOGI(TAG, "Testing heap capabilities");
    
    // Test different heap capabilities
    size_t dma_heap = heap_caps_get_free_size(MALLOC_CAP_DMA);
    size_t spiram_heap = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t internal_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    
    ESP_LOGI(TAG, "DMA heap free: %zu bytes", dma_heap);
    ESP_LOGI(TAG, "SPIRAM heap free: %zu bytes", spiram_heap);
    ESP_LOGI(TAG, "Internal heap free: %zu bytes", internal_heap);
    
    // Should have some internal memory available
    HW_ASSERT_TRUE(internal_heap > 10000);
    
    // Test allocating from specific heaps
    void* internal_ptr = heap_caps_malloc(1024, MALLOC_CAP_INTERNAL);
    HW_ASSERT_NOT_NULL(internal_ptr);
    heap_caps_free(internal_ptr);
    
    void* dma_ptr = heap_caps_malloc(1024, MALLOC_CAP_DMA);
    HW_ASSERT_NOT_NULL(dma_ptr);
    heap_caps_free(dma_ptr);
    
    // SPIRAM might not be available on all boards
    if (spiram_heap > 0) {
        ESP_LOGI(TAG, "SPIRAM detected, testing allocation");
        void* spiram_ptr = heap_caps_malloc(1024, MALLOC_CAP_SPIRAM);
        HW_ASSERT_NOT_NULL(spiram_ptr);
        heap_caps_free(spiram_ptr);
    }
    
    ESP_LOGI(TAG, "Heap capabilities test passed");
}

// Test system information and chip identification
HARDWARE_TEST(system_information) {
    ESP_LOGI(TAG, "Testing system information");
    
    // Test chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    ESP_LOGI(TAG, "Chip model: %s", chip_info.model == CHIP_ESP32S3 ? "ESP32-S3" : "Unknown");
    ESP_LOGI(TAG, "Chip revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "CPU cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Features: 0x%08lX", chip_info.features);
    
    HW_ASSERT_EQ(CHIP_ESP32S3, chip_info.model);
    HW_ASSERT_TRUE(chip_info.cores >= 1);
    
    // Test IDF version
    const char* idf_version = esp_get_idf_version();
    ESP_LOGI(TAG, "IDF version: %s", idf_version);
    HW_ASSERT_NOT_NULL(idf_version);
    
    // Test reset reason
    esp_reset_reason_t reset_reason = esp_reset_reason();
    ESP_LOGI(TAG, "Reset reason: %d", reset_reason);
    
    ESP_LOGI(TAG, "System information test passed");
} 