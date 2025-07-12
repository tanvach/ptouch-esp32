#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "ptouch_esp32.h"
#include "config.h"

// Web server
AsyncWebServer server(WEB_SERVER_PORT);
AsyncWebSocket ws("/ws");

// P-touch printer instance
PtouchPrinter printer;

// Global variables
bool printerConnected = false;
String printerName = "Unknown";
int printerMaxWidth = 0;
int printerTapeWidth = 0;
String printerStatus = "Disconnected";

// Function prototypes
void initWiFi();
void initSPIFFS();
void initWebServer();
void initPrinter();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void broadcastPrinterStatus();
void checkPrinterStatus();
String processorPlaceholder(const String& var);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting P-touch ESP32 Server...");
    
    // Initialize SPIFFS
    initSPIFFS();
    
    // Initialize WiFi
    initWiFi();
    
    // Initialize printer
    initPrinter();
    
    // Initialize web server
    initWebServer();
    
    Serial.println("Setup complete!");
    Serial.print("Web interface: http://");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Check printer status periodically
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > PRINTER_STATUS_CHECK_INTERVAL) {
        checkPrinterStatus();
        lastCheck = millis();
    }
    
    // Clean up WebSocket clients
    ws.cleanupClients();
    
    delay(WS_CLEANUP_INTERVAL);
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
}

void initSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
    Serial.println("SPIFFS mounted successfully");
}

void initPrinter() {
    Serial.println("Initializing P-touch printer...");
    
    printer.setVerbose(PRINTER_VERBOSE);
    
    if (printer.begin()) {
        Serial.println("USB Host initialized");
        
        if (printer.detectPrinter()) {
            if (printer.connect()) {
                printerConnected = true;
                printerName = printer.getPrinterName();
                printerMaxWidth = printer.getMaxWidth();
                printerTapeWidth = printer.getTapeWidth();
                printerStatus = "Connected";
                
                Serial.printf("Printer connected: %s\n", printerName.c_str());
                Serial.printf("Max width: %d px, Tape width: %d px\n", printerMaxWidth, printerTapeWidth);
            } else {
                printerStatus = "Connection failed";
                Serial.println("Failed to connect to printer");
            }
        } else {
            printerStatus = "Not detected";
            Serial.println("No printer detected");
        }
    } else {
        printerStatus = "USB Host init failed";
        Serial.println("Failed to initialize USB Host");
    }
}

void initWebServer() {
    // WebSocket handler
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    
    // Serve static files from SPIFFS
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processorPlaceholder);
    
    // API endpoint - Get printer status
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(1024);
        doc["connected"] = printerConnected;
        doc["name"] = printerName;
        doc["status"] = printerStatus;
        doc["maxWidth"] = printerMaxWidth;
        doc["tapeWidth"] = printerTapeWidth;
        
        if (printerConnected) {
            doc["mediaType"] = printer.getMediaType();
            doc["tapeColor"] = printer.getTapeColor();
            doc["textColor"] = printer.getTextColor();
            doc["hasError"] = printer.hasError();
            if (printer.hasError()) {
                doc["errorDescription"] = printer.getErrorDescription();
            }
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API endpoint - Print text
    server.on("/api/print/text", HTTP_POST, [](AsyncWebServerRequest *request){
        if (!printerConnected) {
            request->send(400, "text/plain", "Printer not connected");
            return;
        }
        
        if (!request->hasParam("text", true)) {
            request->send(400, "text/plain", "Missing text parameter");
            return;
        }
        
        String text = request->getParam("text", true)->value();
        if (text.length() == 0) {
            request->send(400, "text/plain", "Empty text");
            return;
        }
        
        Serial.printf("Printing text: %s\n", text.c_str());
        
        bool success = printer.printText(text.c_str());
        
        if (success) {
            request->send(200, "text/plain", "Print job sent successfully");
        } else {
            request->send(500, "text/plain", "Print job failed");
        }
    });
    
    // API endpoint - Print image (base64 encoded bitmap)
    server.on("/api/print/image", HTTP_POST, [](AsyncWebServerRequest *request){
        if (!printerConnected) {
            request->send(400, "text/plain", "Printer not connected");
            return;
        }
        
        if (!request->hasParam("image", true) || !request->hasParam("width", true) || !request->hasParam("height", true)) {
            request->send(400, "text/plain", "Missing parameters");
            return;
        }
        
        String imageData = request->getParam("image", true)->value();
        int width = request->getParam("width", true)->value().toInt();
        int height = request->getParam("height", true)->value().toInt();
        
        if (width <= 0 || height <= 0) {
            request->send(400, "text/plain", "Invalid dimensions");
            return;
        }
        
        // TODO: Decode base64 image data and print
        // For now, just acknowledge the request
        Serial.printf("Printing image: %dx%d pixels\n", width, height);
        
        request->send(200, "text/plain", "Image print not implemented yet");
    });
    
    // API endpoint - Reconnect printer
    server.on("/api/reconnect", HTTP_POST, [](AsyncWebServerRequest *request){
        Serial.println("Reconnecting printer...");
        printer.disconnect();
        initPrinter();
        broadcastPrinterStatus();
        request->send(200, "text/plain", "Reconnection attempt completed");
    });
    
    // API endpoint - List supported printers
    server.on("/api/printers", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument doc(2048);
        JsonArray printers = doc.createNestedArray("printers");
        
        const pt_dev_info* devices = PtouchPrinter::getSupportedDevices();
        for (int i = 0; devices[i].vid != 0; i++) {
            if (!(devices[i].flags & FLAG_PLITE)) {
                JsonObject printer = printers.createNestedObject();
                printer["name"] = devices[i].name;
                printer["vid"] = devices[i].vid;
                printer["pid"] = devices[i].pid;
                printer["maxWidth"] = devices[i].max_px;
                printer["dpi"] = devices[i].dpi;
            }
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Handle 404
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
    
    server.begin();
    Serial.println("Web server started");
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            broadcastPrinterStatus();
            break;
        
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        String message = "";
        for (size_t i = 0; i < len; i++) {
            message += (char)data[i];
        }
        
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, message);
        
        String command = doc["command"];
        
        if (command == "getStatus") {
            broadcastPrinterStatus();
        } else if (command == "printText") {
            String text = doc["text"];
            if (printerConnected && text.length() > 0) {
                bool success = printer.printText(text.c_str());
                
                DynamicJsonDocument response(256);
                response["type"] = "printResult";
                response["success"] = success;
                
                String responseStr;
                serializeJson(response, responseStr);
                ws.textAll(responseStr);
            }
        } else if (command == "reconnect") {
            printer.disconnect();
            initPrinter();
            broadcastPrinterStatus();
        }
    }
}

void broadcastPrinterStatus() {
    DynamicJsonDocument doc(1024);
    doc["type"] = "printerStatus";
    doc["connected"] = printerConnected;
    doc["name"] = printerName;
    doc["status"] = printerStatus;
    doc["maxWidth"] = printerMaxWidth;
    doc["tapeWidth"] = printerTapeWidth;
    
    if (printerConnected) {
        doc["mediaType"] = printer.getMediaType();
        doc["tapeColor"] = printer.getTapeColor();
        doc["textColor"] = printer.getTextColor();
        doc["hasError"] = printer.hasError();
        if (printer.hasError()) {
            doc["errorDescription"] = printer.getErrorDescription();
        }
    }
    
    String message;
    serializeJson(doc, message);
    ws.textAll(message);
}

void checkPrinterStatus() {
    if (printerConnected) {
        // Try to get status from printer
        if (!printer.getStatus()) {
            // Connection might be lost
            printerConnected = false;
            printerStatus = "Connection lost";
            Serial.println("Printer connection lost");
            broadcastPrinterStatus();
        } else {
            // Update tape width if it changed
            int currentTapeWidth = printer.getTapeWidth();
            if (currentTapeWidth != printerTapeWidth) {
                printerTapeWidth = currentTapeWidth;
                Serial.printf("Tape width changed to: %d px\n", printerTapeWidth);
                broadcastPrinterStatus();
            }
        }
    } else {
        // Try to reconnect
        if (printer.detectPrinter()) {
            if (printer.connect()) {
                printerConnected = true;
                printerName = printer.getPrinterName();
                printerMaxWidth = printer.getMaxWidth();
                printerTapeWidth = printer.getTapeWidth();
                printerStatus = "Connected";
                
                Serial.printf("Printer reconnected: %s\n", printerName.c_str());
                broadcastPrinterStatus();
            }
        }
    }
}

String processorPlaceholder(const String& var) {
    if (var == "PRINTER_NAME") {
        return printerName;
    } else if (var == "PRINTER_STATUS") {
        return printerStatus;
    } else if (var == "WIFI_SSID") {
        return WIFI_SSID;
    } else if (var == "IP_ADDRESS") {
        return WiFi.localIP().toString();
    }
    return String();
} 