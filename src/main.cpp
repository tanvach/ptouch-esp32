/*
 * P-touch ESP32 Label Printer Server - Main Application
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "ArduinoJson.h"

// Include our P-touch library
#include "../lib/ptouch-esp32/include/ptouch_esp32.h"
#include "../include/config.h"

static const char *TAG = "ptouch-server";

// WiFi event group
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
#define ESP_MAXIMUM_RETRY  5

// HTTP server handle
static httpd_handle_t server = NULL;

// P-touch printer instance
static PtouchPrinter *printer = nullptr;

// Global variables for printer status
static bool printerConnected = false;
static char printerName[64] = "Unknown";
static int printerMaxWidth = 0;
static int printerTapeWidth = 0;
static char printerStatus[64] = "Disconnected";

// Function prototypes
static void wifi_init_sta(void);
static esp_err_t start_webserver(void);
static void stop_webserver(void);
static void init_printer(void);
static void printer_status_task(void *pvParameters);

// WiFi event handler
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// WiFi initialization
static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

// HTTP request handlers

// Root handler - serve simple HTML
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char* simple_html = R"(
<!DOCTYPE html>
<html><head><title>P-touch ESP32</title></head>
<body>
<h1>P-touch ESP32 Label Printer</h1>
<p>Web interface temporarily disabled. Use API endpoints directly:</p>
<ul>
<li>GET /api/status - Printer status</li>
<li>POST /api/print/text - Print text (JSON: {"text": "your text"})</li>
<li>POST /api/reconnect - Reconnect printer</li>
<li>GET /api/printers - List supported printers</li>
</ul>
</body></html>
)";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, simple_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// API status endpoint
static esp_err_t api_status_get_handler(httpd_req_t *req)
{
    JsonDocument doc;
    doc["connected"] = printerConnected;
    doc["name"] = printerName;
    doc["status"] = printerStatus;
    doc["maxWidth"] = printerMaxWidth;
    doc["tapeWidth"] = printerTapeWidth;

    if (printerConnected && printer) {
        doc["mediaType"] = printer->getMediaType();
        doc["tapeColor"] = printer->getTapeColor();
        doc["textColor"] = printer->getTextColor();
        doc["hasError"] = printer->hasError();
        if (printer->hasError()) {
            doc["errorDescription"] = printer->getErrorDescription();
        }
    }

    std::string response;
    serializeJson(doc, response);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response.c_str(), response.length());
    return ESP_OK;
}

// API print text endpoint
static esp_err_t api_print_text_post_handler(httpd_req_t *req)
{
    char buf[1024];
    int ret, remaining = req->content_len;

    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Content too long");
        return ESP_FAIL;
    }

    // Receive the content
    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    if (!doc["text"].is<const char*>()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing text parameter");
        return ESP_FAIL;
    }

    const char* text = doc["text"];
    if (!text || strlen(text) == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Empty text");
        return ESP_FAIL;
    }

    if (!printerConnected || !printer) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Printer not connected");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Printing text: %s", text);

    bool success = printer->printText(text);

    if (success) {
        httpd_resp_send(req, "Print job sent successfully", HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Print job failed");
    }

    return ESP_OK;
}

// API reconnect endpoint
static esp_err_t api_reconnect_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Reconnecting printer...");
    
    if (printer) {
        printer->disconnect();
        init_printer();
    }
    
    httpd_resp_send(req, "Reconnection attempt completed", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// API list printers endpoint
static esp_err_t api_printers_get_handler(httpd_req_t *req)
{
    JsonDocument doc;
    JsonArray printers = doc["printers"].to<JsonArray>();
    
    const pt_dev_info* devices = PtouchPrinter::getSupportedDevices();
    for (int i = 0; devices[i].vid != 0; i++) {
        if (!(devices[i].flags & FLAG_PLITE)) {
            JsonObject printer = printers.add<JsonObject>();
            printer["name"] = devices[i].name;
            printer["vid"] = devices[i].vid;
            printer["pid"] = devices[i].pid;
            printer["maxWidth"] = devices[i].max_px;
            printer["dpi"] = devices[i].dpi;
        }
    }
    
    std::string response;
    serializeJson(doc, response);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response.c_str(), response.length());
    return ESP_OK;
}

// Initialize HTTP server
static esp_err_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = WEB_SERVER_PORT;
    config.max_uri_handlers = 16;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");

        // Root handler
        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root);

        // API handlers
        httpd_uri_t api_status = {
            .uri       = "/api/status",
            .method    = HTTP_GET,
            .handler   = api_status_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &api_status);

        httpd_uri_t api_print_text = {
            .uri       = "/api/print/text",
            .method    = HTTP_POST,
            .handler   = api_print_text_post_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &api_print_text);

        httpd_uri_t api_reconnect = {
            .uri       = "/api/reconnect",
            .method    = HTTP_POST,
            .handler   = api_reconnect_post_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &api_reconnect);

        httpd_uri_t api_printers = {
            .uri       = "/api/printers",
            .method    = HTTP_GET,
            .handler   = api_printers_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &api_printers);

        return ESP_OK;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return ESP_FAIL;
}

static void stop_webserver(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

// Initialize printer
static void init_printer(void)
{
    ESP_LOGI(TAG, "Initializing P-touch printer...");
    
    if (!printer) {
        printer = new PtouchPrinter();
    }
    
    printer->setVerbose(PRINTER_VERBOSE);
    
    if (printer->begin()) {
        ESP_LOGI(TAG, "USB Host initialized");
        
        if (printer->detectPrinter()) {
            if (printer->connect()) {
                printerConnected = true;
                strncpy(printerName, printer->getPrinterName(), sizeof(printerName) - 1);
                printerMaxWidth = printer->getMaxWidth();
                printerTapeWidth = printer->getTapeWidth();
                strncpy(printerStatus, "Connected", sizeof(printerStatus) - 1);
                
                ESP_LOGI(TAG, "Printer connected: %s", printerName);
                ESP_LOGI(TAG, "Max width: %d px, Tape width: %d px", printerMaxWidth, printerTapeWidth);
            } else {
                strncpy(printerStatus, "Connection failed", sizeof(printerStatus) - 1);
                ESP_LOGI(TAG, "Failed to connect to printer");
            }
        } else {
            strncpy(printerStatus, "Not detected", sizeof(printerStatus) - 1);
            ESP_LOGI(TAG, "No printer detected");
        }
    } else {
        strncpy(printerStatus, "USB Host init failed", sizeof(printerStatus) - 1);
        ESP_LOGI(TAG, "Failed to initialize USB Host");
    }
}

// Printer status monitoring task
static void printer_status_task(void *pvParameters)
{
    while (1) {
        if (printerConnected && printer) {
            // Try to get status from printer
            if (!printer->getStatus()) {
                // Connection might be lost
                printerConnected = false;
                strncpy(printerStatus, "Connection lost", sizeof(printerStatus) - 1);
                ESP_LOGI(TAG, "Printer connection lost");
            } else {
                // Update tape width if it changed
                int currentTapeWidth = printer->getTapeWidth();
                if (currentTapeWidth != printerTapeWidth) {
                    printerTapeWidth = currentTapeWidth;
                    ESP_LOGI(TAG, "Tape width changed to: %d px", printerTapeWidth);
                }
            }
        } else {
            // Try to reconnect
            if (printer && printer->detectPrinter()) {
                if (printer->connect()) {
                    printerConnected = true;
                    strncpy(printerName, printer->getPrinterName(), sizeof(printerName) - 1);
                    printerMaxWidth = printer->getMaxWidth();
                    printerTapeWidth = printer->getTapeWidth();
                    strncpy(printerStatus, "Connected", sizeof(printerStatus) - 1);
                    
                    ESP_LOGI(TAG, "Printer reconnected: %s", printerName);
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(PRINTER_STATUS_CHECK_INTERVAL));
    }
}

// Initialize SPIFFS
static void init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

// Main application entry point
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting P-touch ESP32 Server...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize SPIFFS
    init_spiffs();

    // Initialize WiFi
    wifi_init_sta();

    // Initialize printer
    init_printer();

    // Start web server
    start_webserver();

    // Start printer status monitoring task
    xTaskCreate(printer_status_task, "printer_status", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Setup complete!");

    // Get IP address
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            ESP_LOGI(TAG, "Web interface: http://" IPSTR, IP2STR(&ip_info.ip));
        }
    }

    // Keep the app running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
} 