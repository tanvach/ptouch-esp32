#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
// Copy this file to config.h and update with your actual credentials
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";

// Web Server Configuration
const int WEB_SERVER_PORT = 80;

// Printer Configuration
const bool PRINTER_VERBOSE = true;
const int PRINTER_STATUS_CHECK_INTERVAL = 5000;  // milliseconds

// WebSocket Configuration
const int WS_CLEANUP_INTERVAL = 100;  // milliseconds

#endif // CONFIG_H 