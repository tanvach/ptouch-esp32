#include "hardware_test_runner.h"
#include "usb/usb_host.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char* TAG = "USB_HOST_HW_TEST";

// Test USB Host library installation and uninstallation
USB_HOST_TEST(host_library_lifecycle) {
    ESP_LOGI(TAG, "Testing USB Host library lifecycle");
    
    // Test installation
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    
    // Test library info
    usb_host_lib_info_t lib_info;
    HW_ASSERT_ESP_OK(usb_host_lib_info(&lib_info));
    HW_ASSERT_EQ(0, lib_info.num_devices);
    HW_ASSERT_EQ(0, lib_info.num_clients);
    
    // Test uninstallation
    HW_ASSERT_ESP_OK(usb_host_uninstall());
}

// Test client registration and deregistration
USB_HOST_TEST(client_registration) {
    ESP_LOGI(TAG, "Testing USB Host client registration");
    
    // Install host library first
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    
    // Test client registration
    usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = nullptr,
            .callback_arg = nullptr,
        }
    };
    
    usb_host_client_handle_t client_hdl;
    HW_ASSERT_ESP_OK(usb_host_client_register(&client_config, &client_hdl));
    
    // Test library info with client
    usb_host_lib_info_t lib_info;
    HW_ASSERT_ESP_OK(usb_host_lib_info(&lib_info));
    HW_ASSERT_EQ(1, lib_info.num_clients);
    
    // Test client deregistration
    HW_ASSERT_ESP_OK(usb_host_client_deregister(client_hdl));
    
    // Cleanup
    HW_ASSERT_ESP_OK(usb_host_uninstall());
}

// Test root port power management (if supported)
USB_HOST_TEST(root_port_power_management) {
    ESP_LOGI(TAG, "Testing USB Host root port power management");
    
    // Install with unpowered root port
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = true,  // Start with unpowered port
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    
    // Test setting root port power
    HW_ASSERT_ESP_OK(usb_host_lib_set_root_port_power(true));
    vTaskDelay(pdMS_TO_TICKS(100));  // Allow time for power to stabilize
    
    HW_ASSERT_ESP_OK(usb_host_lib_set_root_port_power(false));
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Cleanup
    HW_ASSERT_ESP_OK(usb_host_uninstall());
}

// Test device enumeration with no devices connected
USB_HOST_TEST(device_enumeration_empty) {
    ESP_LOGI(TAG, "Testing USB Host device enumeration (no devices)");
    
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    
    // Test device address list fill with no devices
    uint8_t dev_addr_list[10];
    int num_devices;
    HW_ASSERT_ESP_OK(usb_host_device_addr_list_fill(10, dev_addr_list, &num_devices));
    HW_ASSERT_EQ(0, num_devices);
    
    // Cleanup
    HW_ASSERT_ESP_OK(usb_host_uninstall());
}

// Test transfer allocation and deallocation
USB_HOST_TEST(transfer_allocation) {
    ESP_LOGI(TAG, "Testing USB Host transfer allocation");
    
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    
    // Test transfer allocation
    usb_transfer_t *transfer;
    HW_ASSERT_ESP_OK(usb_host_transfer_alloc(1024, 0, &transfer));
    HW_ASSERT_NOT_NULL(transfer);
    HW_ASSERT_NOT_NULL(transfer->data_buffer);
    HW_ASSERT_GTE(transfer->data_buffer_size, 1024);
    
    // Test transfer deallocation
    HW_ASSERT_ESP_OK(usb_host_transfer_free(transfer));
    
    // Test NULL transfer free (should succeed)
    HW_ASSERT_ESP_OK(usb_host_transfer_free(nullptr));
    
    // Cleanup
    HW_ASSERT_ESP_OK(usb_host_uninstall());
}

// Test USB Host timing and performance
USB_HOST_TEST(host_timing_performance) {
    ESP_LOGI(TAG, "Testing USB Host timing and performance");
    
    int64_t start_time = esp_timer_get_time();
    
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    // Measure install time
    int64_t install_start = esp_timer_get_time();
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    int64_t install_time = esp_timer_get_time() - install_start;
    
    ESP_LOGI(TAG, "USB Host install time: %lld us", install_time);
    HW_ASSERT_LT(install_time, 100000);  // Should complete within 100ms
    
    // Measure transfer allocation time
    int64_t alloc_start = esp_timer_get_time();
    usb_transfer_t *transfer;
    HW_ASSERT_ESP_OK(usb_host_transfer_alloc(1024, 0, &transfer));
    int64_t alloc_time = esp_timer_get_time() - alloc_start;
    
    ESP_LOGI(TAG, "Transfer allocation time: %lld us", alloc_time);
    HW_ASSERT_LT(alloc_time, 10000);  // Should complete within 10ms
    
    // Cleanup
    HW_ASSERT_ESP_OK(usb_host_transfer_free(transfer));
    HW_ASSERT_ESP_OK(usb_host_uninstall());
    
    int64_t total_time = esp_timer_get_time() - start_time;
    ESP_LOGI(TAG, "Total test time: %lld us", total_time);
}

// Test memory usage during USB operations
USB_HOST_TEST(memory_usage_monitoring) {
    ESP_LOGI(TAG, "Testing USB Host memory usage");
    
    size_t initial_free = esp_get_free_heap_size();
    size_t initial_min = esp_get_minimum_free_heap_size();
    
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    
    size_t after_install = esp_get_free_heap_size();
    size_t install_usage = initial_free - after_install;
    ESP_LOGI(TAG, "USB Host install memory usage: %zu bytes", install_usage);
    
    // Allocate multiple transfers to test memory usage
    usb_transfer_t *transfers[5];
    for (int i = 0; i < 5; i++) {
        HW_ASSERT_ESP_OK(usb_host_transfer_alloc(1024, 0, &transfers[i]));
    }
    
    size_t after_transfers = esp_get_free_heap_size();
    size_t transfer_usage = after_install - after_transfers;
    ESP_LOGI(TAG, "Transfer allocation memory usage: %zu bytes", transfer_usage);
    
    // Free transfers
    for (int i = 0; i < 5; i++) {
        HW_ASSERT_ESP_OK(usb_host_transfer_free(transfers[i]));
    }
    
    // Cleanup
    HW_ASSERT_ESP_OK(usb_host_uninstall());
    
    size_t final_free = esp_get_free_heap_size();
    size_t leaked = initial_free - final_free;
    
    ESP_LOGI(TAG, "Memory leak check: %zu bytes", leaked);
    HW_ASSERT_LT(leaked, 1024);  // Should not leak more than 1KB
}

// Test error handling scenarios
USB_HOST_TEST(error_handling_scenarios) {
    ESP_LOGI(TAG, "Testing USB Host error handling");
    
    // Test invalid parameters
    HW_ASSERT_ESP_ERR(ESP_ERR_INVALID_ARG, usb_host_install(nullptr));
    
    // Test operations without installation
    usb_host_lib_info_t lib_info;
    HW_ASSERT_ESP_ERR(ESP_ERR_INVALID_STATE, usb_host_lib_info(&lib_info));
    
    // Test double installation
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .root_port_unpowered = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
        .enum_filter_cb = NULL
    };
    
    HW_ASSERT_ESP_OK(usb_host_install(&host_config));
    HW_ASSERT_ESP_ERR(ESP_ERR_INVALID_STATE, usb_host_install(&host_config));
    
    // Test client registration with invalid config
    usb_host_client_handle_t client_hdl;
    HW_ASSERT_ESP_ERR(ESP_ERR_INVALID_ARG, usb_host_client_register(nullptr, &client_hdl));
    
    // Cleanup
    HW_ASSERT_ESP_OK(usb_host_uninstall());
} 