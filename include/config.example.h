#ifndef CONFIG_H
#define CONFIG_H

#include "ptouch_debug.h"  // For debug level constants

// WiFi Configuration
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

// Web server configuration
#define WEB_SERVER_PORT 80

// USB Debug configuration
#define ENABLE_USB_DEBUG false
#define USB_DEBUG_LEVEL PTOUCH_DEBUG_LEVEL_INFO  // NONE, ERROR, WARN, INFO, DEBUG, VERBOSE

#endif // CONFIG_H 