#ifndef MOCK_USB_HOST_H
#define MOCK_USB_HOST_H

#include "hardware_abstraction.h"
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <functional>

// Mock device handle - just a simple pointer wrapper
struct MockDeviceHandle {
    uint8_t device_address;
    uint16_t vid;
    uint16_t pid;
    bool interface_claimed = false;
    bool is_open = false;
};

// USB transfer record for verification
struct USBTransferRecord {
    uint8_t endpoint;
    std::vector<uint8_t> data;
    size_t length;
    uint32_t timeout_ms;
    esp_err_t result;
    size_t actual_length;
    uint64_t timestamp;
    
    USBTransferRecord(uint8_t ep, const uint8_t* buf, size_t len, uint32_t timeout) 
        : endpoint(ep), data(buf, buf + len), length(len), timeout_ms(timeout), 
          result(ESP_OK), actual_length(len), timestamp(0) {}
};

// Mock USB Host implementation
class MockUSBHost : public IUSBHost {
private:
    bool host_driver_installed = false;
    bool client_registered = false;
    
    // Mock devices
    std::map<uint8_t, std::unique_ptr<MockDeviceHandle>> mock_devices;
    uint8_t next_device_address = 1;
    
    // Transfer logging
    std::vector<USBTransferRecord> sent_transfers;
    std::vector<USBTransferRecord> received_transfers;
    
    // Response queues for mocking received data
    std::queue<std::vector<uint8_t>> bulk_in_responses;
    std::queue<esp_err_t> next_transfer_errors;
    
    // Simulation settings
    uint32_t simulated_transfer_delay_ms = 0;
    bool fail_next_operation = false;
    esp_err_t next_operation_error = ESP_FAIL;
    
    // Statistics
    size_t total_transfers = 0;
    size_t failed_transfers = 0;
    
public:
    MockUSBHost() = default;
    virtual ~MockUSBHost() = default;
    
    // IUSBHost implementation
    esp_err_t install_host_driver() override {
        if (fail_next_operation) {
            fail_next_operation = false;
            return next_operation_error;
        }
        
        if (host_driver_installed) {
            return ESP_ERR_INVALID_ARG; // Already installed
        }
        
        host_driver_installed = true;
        return ESP_OK;
    }
    
    esp_err_t uninstall_host_driver() override {
        if (!host_driver_installed) {
            return ESP_ERR_INVALID_ARG;
        }
        
        // Clean up all devices
        mock_devices.clear();
        host_driver_installed = false;
        client_registered = false;
        
        return ESP_OK;
    }
    
    esp_err_t register_client() override {
        if (!host_driver_installed) {
            return ESP_ERR_INVALID_ARG;
        }
        
        if (client_registered) {
            return ESP_ERR_INVALID_ARG;
        }
        
        client_registered = true;
        return ESP_OK;
    }
    
    esp_err_t deregister_client() override {
        if (!client_registered) {
            return ESP_ERR_INVALID_ARG;
        }
        
        client_registered = false;
        return ESP_OK;
    }
    
    esp_err_t get_device_list(std::vector<uint8_t>& device_addresses) override {
        if (!client_registered) {
            return ESP_ERR_INVALID_ARG;
        }
        
        device_addresses.clear();
        for (const auto& pair : mock_devices) {
            device_addresses.push_back(pair.first);
        }
        
        return ESP_OK;
    }
    
    esp_err_t device_open(uint8_t device_addr, void** device_handle) override {
        if (!client_registered) {
            return ESP_ERR_INVALID_ARG;
        }
        
        auto it = mock_devices.find(device_addr);
        if (it == mock_devices.end()) {
            return ESP_ERR_NOT_FOUND;
        }
        
        if (it->second->is_open) {
            return ESP_ERR_INVALID_ARG; // Already open
        }
        
        it->second->is_open = true;
        *device_handle = it->second.get();
        return ESP_OK;
    }
    
    esp_err_t device_close(void* device_handle) override {
        MockDeviceHandle* handle = static_cast<MockDeviceHandle*>(device_handle);
        if (!handle || !handle->is_open) {
            return ESP_ERR_INVALID_ARG;
        }
        
        handle->is_open = false;
        handle->interface_claimed = false;
        return ESP_OK;
    }
    
    esp_err_t get_device_descriptor(void* device_handle, uint16_t& vid, uint16_t& pid) override {
        MockDeviceHandle* handle = static_cast<MockDeviceHandle*>(device_handle);
        if (!handle || !handle->is_open) {
            return ESP_ERR_INVALID_ARG;
        }
        
        vid = handle->vid;
        pid = handle->pid;
        return ESP_OK;
    }
    
    esp_err_t claim_interface(void* device_handle, uint8_t interface_num) override {
        MockDeviceHandle* handle = static_cast<MockDeviceHandle*>(device_handle);
        if (!handle || !handle->is_open) {
            return ESP_ERR_INVALID_ARG;
        }
        
        if (handle->interface_claimed) {
            return ESP_ERR_INVALID_ARG; // Already claimed
        }
        
        handle->interface_claimed = true;
        (void)interface_num; // We don't track specific interface numbers in this mock
        return ESP_OK;
    }
    
    esp_err_t release_interface(void* device_handle, uint8_t interface_num) override {
        MockDeviceHandle* handle = static_cast<MockDeviceHandle*>(device_handle);
        if (!handle || !handle->is_open) {
            return ESP_ERR_INVALID_ARG;
        }
        
        handle->interface_claimed = false;
        (void)interface_num; // We don't track specific interface numbers in this mock
        return ESP_OK;
    }
    
    esp_err_t bulk_transfer(void* device_handle, uint8_t endpoint, 
                          const uint8_t* data, size_t length, 
                          size_t& actual_length, uint32_t timeout_ms) override {
        
        MockDeviceHandle* handle = static_cast<MockDeviceHandle*>(device_handle);
        if (!handle || !handle->is_open || !handle->interface_claimed) {
            return ESP_ERR_INVALID_ARG;
        }
        
        total_transfers++;
        
        // Check for error injection
        if (!next_transfer_errors.empty()) {
            esp_err_t error = next_transfer_errors.front();
            next_transfer_errors.pop();
            if (error != ESP_OK) {
                failed_transfers++;
                actual_length = 0;
                return error;
            }
        }
        
        // Simulate transfer delay
        if (simulated_transfer_delay_ms > 0) {
            // In a real implementation, you might use std::this_thread::sleep_for
            // For testing, we just record that a delay would occur
        }
        
        if (endpoint & 0x80) {
            // IN transfer (device to host)
            if (bulk_in_responses.empty()) {
                // No response queued - simulate timeout or no data
                actual_length = 0;
                return ESP_ERR_TIMEOUT;
            }
            
            auto response = bulk_in_responses.front();
            bulk_in_responses.pop();
            
            size_t copy_length = std::min(length, response.size());
            if (data) {
                // This is a bit tricky - for IN transfers, we need to write to the buffer
                // But the data parameter is const. In a real mock, you'd handle this differently
                // For now, we'll assume the caller will handle reading the response
            }
            
            actual_length = copy_length;
            
            // Record the transfer
            USBTransferRecord record(endpoint, response.data(), copy_length, timeout_ms);
            record.actual_length = copy_length;
            received_transfers.push_back(record);
            
        } else {
            // OUT transfer (host to device)
            actual_length = length;
            
            // Record the sent data
            USBTransferRecord record(endpoint, data, length, timeout_ms);
            record.actual_length = length;
            sent_transfers.push_back(record);
        }
        
        return ESP_OK;
    }
    
    esp_err_t control_transfer(void* device_handle, uint8_t request_type,
                             uint8_t request, uint16_t value, uint16_t index,
                             uint8_t* data, uint16_t length, uint32_t timeout_ms) override {
        (void)device_handle;
        (void)request_type;
        (void)request;
        (void)value;
        (void)index;
        (void)data;
        (void)length;
        (void)timeout_ms;
        return ESP_OK;
    }
    
    // Mock control methods
    
    // Add a mock device to the USB bus
    uint8_t add_mock_device(uint16_t vid, uint16_t pid) {
        uint8_t addr = next_device_address++;
        auto device = std::make_unique<MockDeviceHandle>();
        device->device_address = addr;
        device->vid = vid;
        device->pid = pid;
        device->is_open = false;
        device->interface_claimed = false;
        
        mock_devices[addr] = std::move(device);
        return addr;
    }
    
    // Remove a mock device
    void remove_mock_device(uint8_t device_addr) {
        mock_devices.erase(device_addr);
    }
    
    // Queue a response for the next IN transfer
    void queue_bulk_in_response(const std::vector<uint8_t>& response) {
        bulk_in_responses.push(response);
    }
    
    // Queue multiple responses
    void queue_bulk_in_responses(const std::vector<std::vector<uint8_t>>& responses) {
        for (const auto& response : responses) {
            bulk_in_responses.push(response);
        }
    }
    
    // Set the next transfer to fail with specified error
    void set_next_transfer_error(esp_err_t error) {
        next_transfer_errors.push(error);
    }
    
    // Set operation failure for testing error handling
    void set_next_operation_failure(esp_err_t error) {
        fail_next_operation = true;
        next_operation_error = error;
    }
    
    // Set simulated transfer delay
    void set_transfer_delay(uint32_t delay_ms) {
        simulated_transfer_delay_ms = delay_ms;
    }
    
    // Get sent transfers for verification
    const std::vector<USBTransferRecord>& get_sent_transfers() const {
        return sent_transfers;
    }
    
    // Get received transfers for verification  
    const std::vector<USBTransferRecord>& get_received_transfers() const {
        return received_transfers;
    }
    
    // Clear transfer history
    void clear_transfer_history() {
        sent_transfers.clear();
        received_transfers.clear();
    }
    
    // Get statistics
    size_t get_total_transfers() const { return total_transfers; }
    size_t get_failed_transfers() const { return failed_transfers; }
    double get_failure_rate() const { 
        return total_transfers > 0 ? (double)failed_transfers / total_transfers : 0.0; 
    }
    
    // Reset all state
    void reset() {
        uninstall_host_driver();
        mock_devices.clear();
        sent_transfers.clear();
        received_transfers.clear();
        
        // Clear queues
        while (!bulk_in_responses.empty()) bulk_in_responses.pop();
        while (!next_transfer_errors.empty()) next_transfer_errors.pop();
        
        // Reset settings
        simulated_transfer_delay_ms = 0;
        fail_next_operation = false;
        next_operation_error = ESP_FAIL;
        total_transfers = 0;
        failed_transfers = 0;
        next_device_address = 1;
    }
    
    // Helper methods for common test scenarios
    
    // Add a Brother P-touch printer device
    uint8_t add_ptouch_printer(uint16_t pid = 0x20e0) { // PT-D460BT by default
        return add_mock_device(0x04F9, pid); // Brother VID
    }
    
    // Queue a typical status response
    void queue_status_response(uint8_t tape_width_mm = 12, uint8_t media_type = 1) {
        std::vector<uint8_t> status = {
            0x80, 0x20, 0x42, 0x30, // Header: printheadmark, size, "B", "0" 
            0x01, 0x00, 0x00, 0x00, // model, country, reserved
            0x00, 0x00,             // error (no error)
            tape_width_mm,          // media width
            media_type,             // media type
            0x00, 0x00, 0x00, 0x00, // ncol, fonts, jp_fonts, mode
            0x00, 0x00,             // density, media_len
            0x00, 0x00, 0x00, 0x00, // status_type, phase_type, phase_number
            0x00, 0x00,             // notif_number, exp
            0x01, 0x00,             // tape_color, text_color
            0x00, 0x00, 0x00, 0x00, // hw_setting
            0x00, 0x00              // reserved
        };
        queue_bulk_in_response(status);
    }
    
    // Queue an ACK response (simple acknowledgment)
    void queue_ack_response() {
        queue_bulk_in_response({0x06}); // ACK
    }
};

#endif // MOCK_USB_HOST_H 