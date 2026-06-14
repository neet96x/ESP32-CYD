#ifndef CYD_CONFIG_H
#define CYD_CONFIG_H

// Copy this file to CYD_Config.h and fill in your local credentials.
// CYD_Config.h is intentionally ignored by git.

static const char* LE_E8_CA = "";

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* API_URL     = "https://api.maxplus-ai.cc/v1/me";
const char* API_KEY     = "YOUR_MAXPLUS_API_KEY";
const unsigned long REFRESH_INTERVAL_MS = 60000;

// MQTT Pager
const char* MQTT_HOST = "YOUR_MQTT_HOST";
const int   MQTT_PORT = 1883;
const char* MQTT_USER = "YOUR_MQTT_USER";
const char* MQTT_PASS = "YOUR_MQTT_PASSWORD";
const char* MQTT_TOPIC = "cyd/pager";

// RGB LED pins on the back of the CYD board.
#define RGB_RED   4
#define RGB_GREEN 16
#define RGB_BLUE  17

// UI colors in RGB565.
#define CYD_PANEL_BLUE 0x03EF
#define CYD_YELLOW     0xFFE0
#define CYD_PINK       0xFBEF
#define CYD_TEXT_BLUE  0x05FF

#endif
