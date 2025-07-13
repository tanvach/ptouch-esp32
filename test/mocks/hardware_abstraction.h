#ifndef HARDWARE_ABSTRACTION_H
#define HARDWARE_ABSTRACTION_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

// ESP-IDF error codes - simplified for testing
typedef int esp_err_t;
#define ESP_OK          0
#define ESP_FAIL        -1
#define ESP_ERR_NO_MEM  0x101
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_TIMEOUT 0x103
#define ESP_ERR_NOT_FOUND 0x105

// Mock ESP logging
#define ESP_LOGI(tag, format, ...) printf("[INFO] " tag ": " format "\n", ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...) printf("[ERROR] " tag ": " format "\n", ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf("[WARN] " tag ": " format "\n", ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) printf("[DEBUG] " tag ": " format "\n", ##__VA_ARGS__)

// USB Host abstraction interface
class IUSBHost {
public:
    virtual ~IUSBHost() = default;
    
    // Device management
    virtual esp_err_t install_host_driver() = 0;
    virtual esp_err_t uninstall_host_driver() = 0;
    virtual esp_err_t register_client() = 0;
    virtual esp_err_t deregister_client() = 0;
    
    // Device operations
    virtual esp_err_t get_device_list(std::vector<uint8_t>& device_addresses) = 0;
    virtual esp_err_t device_open(uint8_t device_addr, void** device_handle) = 0;
    virtual esp_err_t device_close(void* device_handle) = 0;
    virtual esp_err_t get_device_descriptor(void* device_handle, uint16_t& vid, uint16_t& pid) = 0;
    
    // Interface operations
    virtual esp_err_t claim_interface(void* device_handle, uint8_t interface_num) = 0;
    virtual esp_err_t release_interface(void* device_handle, uint8_t interface_num) = 0;
    
    // Transfer operations
    virtual esp_err_t bulk_transfer(void* device_handle, uint8_t endpoint, 
                                  const uint8_t* data, size_t length, 
                                  size_t& actual_length, uint32_t timeout_ms) = 0;
    virtual esp_err_t control_transfer(void* device_handle, uint8_t request_type,
                                     uint8_t request, uint16_t value, uint16_t index,
                                     uint8_t* data, uint16_t length, uint32_t timeout_ms) = 0;
};

// WiFi abstraction interface  
class IWiFi {
public:
    virtual ~IWiFi() = default;
    
    virtual esp_err_t init() = 0;
    virtual esp_err_t deinit() = 0;
    virtual esp_err_t connect(const std::string& ssid, const std::string& password) = 0;
    virtual esp_err_t disconnect() = 0;
    virtual bool is_connected() = 0;
    virtual std::string get_ip_address() = 0;
};

// HTTP Server abstraction interface
class IHTTPServer {
public:
    virtual ~IHTTPServer() = default;
    
    struct Request {
        std::string uri;
        std::string method;
        std::vector<uint8_t> content;
        std::string content_type;
    };
    
    struct Response {
        int status_code = 200;
        std::vector<uint8_t> content;
        std::string content_type = "text/plain";
    };
    
    using RequestHandler = std::function<Response(const Request&)>;
    
    virtual esp_err_t start(uint16_t port) = 0;
    virtual esp_err_t stop() = 0;
    virtual esp_err_t register_handler(const std::string& uri, const std::string& method, RequestHandler handler) = 0;
    virtual bool is_running() = 0;
};

// System abstraction interface
class ISystem {
public:
    virtual ~ISystem() = default;
    
    virtual uint32_t get_time_ms() = 0;
    virtual void delay_ms(uint32_t delay) = 0;
    virtual void* malloc(size_t size) = 0;
    virtual void free(void* ptr) = 0;
    virtual size_t get_free_heap_size() = 0;
};

// Main hardware abstraction factory
class HardwareAbstraction {
public:
    virtual ~HardwareAbstraction() = default;
    
    virtual IUSBHost* get_usb_host() = 0;
    virtual IWiFi* get_wifi() = 0;
    virtual IHTTPServer* get_http_server() = 0;
    virtual ISystem* get_system() = 0;
};

// Global hardware abstraction instance (dependency injection)
extern HardwareAbstraction* g_hardware;

// Convenience macros for accessing hardware
#define USB_HOST() (g_hardware->get_usb_host())
#define WIFI() (g_hardware->get_wifi())
#define HTTP_SERVER() (g_hardware->get_http_server())
#define SYSTEM() (g_hardware->get_system())

#endif // HARDWARE_ABSTRACTION_H 