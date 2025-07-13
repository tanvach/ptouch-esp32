#include "../runner/test_runner.h"
#include "../mocks/mock_usb_host.h"
#include "../fixtures/test_data.h"
#include "../fixtures/test_helpers.h"

// Test USB Host driver installation
TEST(USBHostDriverInstallation) {
    MockUSBHost usb;
    
    // Test installation
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    
    // Test double installation should fail
    ASSERT_EQ(ESP_ERR_INVALID_ARG, usb.install_host_driver());
    
    // Cleanup
    ASSERT_EQ(ESP_OK, usb.uninstall_host_driver());
}

// Test USB Host driver uninstallation
TEST(USBHostDriverUninstallation) {
    MockUSBHost usb;
    
    // Install first
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    
    // Test uninstallation
    ASSERT_EQ(ESP_OK, usb.uninstall_host_driver());
    
    // Test double uninstallation should fail
    ASSERT_EQ(ESP_ERR_INVALID_ARG, usb.uninstall_host_driver());
}

// Test USB client registration
TEST(USBClientRegistration) {
    MockUSBHost usb;
    
    // Should fail without driver
    ASSERT_EQ(ESP_ERR_INVALID_ARG, usb.register_client());
    
    // Install driver first
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    
    // Now registration should succeed
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Double registration should fail
    ASSERT_EQ(ESP_ERR_INVALID_ARG, usb.register_client());
    
    // Cleanup
    ASSERT_EQ(ESP_OK, usb.deregister_client());
    ASSERT_EQ(ESP_OK, usb.uninstall_host_driver());
}

// Test P-touch printer discovery
TEST(PtouchPrinterDiscovery) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add a P-touch printer
    uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);
    
    // Get device list
    std::vector<uint8_t> device_list;
    ASSERT_EQ(ESP_OK, usb.get_device_list(device_list));
    ASSERT_EQ(1u, device_list.size());
    ASSERT_EQ(device_addr, device_list[0]);
    
    // Cleanup
    usb.reset();
}

// Test USB device opening and closing
TEST(USBDeviceOpenClose) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add a device
    uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);
    
    // Open device
    void* handle = nullptr;
    ASSERT_EQ(ESP_OK, usb.device_open(device_addr, &handle));
    ASSERT_NE(nullptr, handle);
    
    // Close device
    ASSERT_EQ(ESP_OK, usb.device_close(handle));
    
    // Cleanup
    usb.reset();
}

// Test USB device descriptor reading
TEST(USBDeviceDescriptor) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add a device and open it
    uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);
    void* handle = nullptr;
    ASSERT_EQ(ESP_OK, usb.device_open(device_addr, &handle));
    
    // Get device descriptor
    uint16_t vid, pid;
    ASSERT_EQ(ESP_OK, usb.get_device_descriptor(handle, vid, pid));
    ASSERT_EQ(TestData::BROTHER_VID, vid);
    ASSERT_EQ(TestData::PT_P700_PID, pid);
    
    // Close device
    ASSERT_EQ(ESP_OK, usb.device_close(handle));
    
    // Cleanup
    usb.reset();
}

// Test USB interface claiming
TEST(USBInterfaceClaiming) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add a device and open it
    uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);
    void* handle = nullptr;
    ASSERT_EQ(ESP_OK, usb.device_open(device_addr, &handle));
    
    // Claim interface
    ASSERT_EQ(ESP_OK, usb.claim_interface(handle, 0));
    
    // Release interface
    ASSERT_EQ(ESP_OK, usb.release_interface(handle, 0));
    
    // Close device
    ASSERT_EQ(ESP_OK, usb.device_close(handle));
    
    // Cleanup
    usb.reset();
}

// Test USB bulk transfer OUT
TEST(USBBulkTransferOut) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add a device and open it
    uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);
    void* handle = nullptr;
    ASSERT_EQ(ESP_OK, usb.device_open(device_addr, &handle));
    ASSERT_EQ(ESP_OK, usb.claim_interface(handle, 0));
    
    // Prepare test data
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04};
    size_t actual_length = 0;
    
    // Perform bulk transfer (OUT endpoint)
    ASSERT_EQ(ESP_OK, usb.bulk_transfer(handle, 0x02, // OUT endpoint
                                       test_data.data(), test_data.size(),
                                       actual_length, 1000));
    
    ASSERT_EQ(test_data.size(), actual_length);
    
    // Check transfer was recorded
    auto transfers = usb.get_sent_transfers();
    ASSERT_EQ(1u, transfers.size());
    
    // Check transfer data matches
    ASSERT_TRUE(TestHelpers::vectors_equal(test_data, transfers[0].data));
    
    // Close device
    ASSERT_EQ(ESP_OK, usb.device_close(handle));
    
    // Cleanup
    usb.reset();
}

// Test USB error injection
TEST(USBErrorInjection) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add a device and open it
    uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);
    void* handle = nullptr;
    ASSERT_EQ(ESP_OK, usb.device_open(device_addr, &handle));
    ASSERT_EQ(ESP_OK, usb.claim_interface(handle, 0));
    
    // Inject error for next transfer
    usb.set_next_transfer_error(ESP_ERR_TIMEOUT);
    
    // Try bulk transfer - should fail
    std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04};
    size_t actual_length = 0;
    
    ASSERT_EQ(ESP_ERR_TIMEOUT, usb.bulk_transfer(handle, 0x02,
                                                test_data.data(), test_data.size(),
                                                actual_length, 1000));
    
    ASSERT_EQ(0u, actual_length);
    
    // Check statistics
    ASSERT_EQ(1u, usb.get_total_transfers());
    ASSERT_EQ(1u, usb.get_failed_transfers());
    
    // Close device
    ASSERT_EQ(ESP_OK, usb.device_close(handle));
    
    // Cleanup
    usb.reset();
}

// Test multiple device management
TEST(MultipleDeviceManagement) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add multiple devices
    uint8_t device1 = usb.add_ptouch_printer(TestData::PT_P700_PID);
    uint8_t device2 = usb.add_ptouch_printer(TestData::PT_H500_PID);
    uint8_t device3 = usb.add_ptouch_printer(TestData::PT_D460BT_PID);
    
    // Check device count
    std::vector<uint8_t> device_list;
    ASSERT_EQ(ESP_OK, usb.get_device_list(device_list));
    ASSERT_EQ(3u, device_list.size());
    
    // Remove one device
    usb.remove_mock_device(device1);
    
    // Check device count decreased
    ASSERT_EQ(ESP_OK, usb.get_device_list(device_list));
    ASSERT_EQ(2u, device_list.size());
    
    // Remove remaining devices
    usb.remove_mock_device(device2);
    usb.remove_mock_device(device3);
    
    ASSERT_EQ(ESP_OK, usb.get_device_list(device_list));
    ASSERT_EQ(0u, device_list.size());
    
    // Cleanup
    usb.reset();
}

// Test USB transfer logging and P-touch protocol
TEST(USBTransferProtocolLogging) {
    MockUSBHost usb;
    ASSERT_EQ(ESP_OK, usb.install_host_driver());
    ASSERT_EQ(ESP_OK, usb.register_client());
    
    // Add a device and open it
    uint8_t device_addr = usb.add_ptouch_printer(TestData::PT_P700_PID);
    void* handle = nullptr;
    ASSERT_EQ(ESP_OK, usb.device_open(device_addr, &handle));
    ASSERT_EQ(ESP_OK, usb.claim_interface(handle, 0));
    
    // Clear any existing transfers
    usb.clear_transfer_history();
    
    // Perform multiple transfers using P-touch protocol commands
    size_t actual_length = 0;
    
    // Send initialization command
    ASSERT_EQ(ESP_OK, usb.bulk_transfer(handle, 0x02, 
                                       TestData::INIT_COMMAND.data(), 
                                       TestData::INIT_COMMAND.size(),
                                       actual_length, 1000));
    
    // Send status request
    ASSERT_EQ(ESP_OK, usb.bulk_transfer(handle, 0x02, 
                                       TestData::STATUS_REQUEST.data(), 
                                       TestData::STATUS_REQUEST.size(),
                                       actual_length, 1000));
    
    // Send compression enable
    ASSERT_EQ(ESP_OK, usb.bulk_transfer(handle, 0x02, 
                                       TestData::PACKBITS_ENABLE.data(), 
                                       TestData::PACKBITS_ENABLE.size(),
                                       actual_length, 1000));
    
    // Check transfer logging
    auto transfers = usb.get_sent_transfers();
    ASSERT_EQ(3u, transfers.size());
    
    // Verify transfer data
    ASSERT_TRUE(TestHelpers::vectors_equal(TestData::INIT_COMMAND, transfers[0].data));
    ASSERT_TRUE(TestHelpers::vectors_equal(TestData::STATUS_REQUEST, transfers[1].data));
    ASSERT_TRUE(TestHelpers::vectors_equal(TestData::PACKBITS_ENABLE, transfers[2].data));
    
    // Clear transfers
    usb.clear_transfer_history();
    ASSERT_EQ(0u, usb.get_sent_transfers().size());
    
    // Close device
    ASSERT_EQ(ESP_OK, usb.device_close(handle));
    
    // Cleanup
    usb.reset();
} 