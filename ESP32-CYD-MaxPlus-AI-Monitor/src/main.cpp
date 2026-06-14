/*
 * CYD AI Dynamic Monitor & Credit Display (TFT_eSPI Version)
 * Hardware: ESP32-2432S028R (CYD / HW-458)
 * Framework: PlatformIO + TFT_eSPI
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "CYD_Config.h"
#include <tcUnicodeTFT_eSPI.h>
#include "NotoSansThai16_tc.h"
#define TFT_NAVY      0x000F
#define TFT_DARKBLUE  0x000A
#define TFT_NIGHTBLUE 0x0005
#define TFT_MIDBLUE 0x0010
TFT_eSPI tft = TFT_eSPI();
UnicodeFontHandler fontHandler(newTFT_eSPITextPipeline(&tft), ENCMODE_UTF8);

WiFiClient mqttWifi;
PubSubClient mqtt(mqttWifi);

unsigned long lastRefresh    = 0;
String lastCreditValue       = "0.00";
String displayName           = "Hello World";
bool wifiConnected           = false;
unsigned long wifiReconnect  = 0;

// Pager feed — 4 บรรทัด ข้อความใหม่ดันขึ้น
#define FEED_LINES 8
String feed[FEED_LINES];

#define THAI_ASCENT 17  // NotoSansThai 16px ascent

// --- วาด icon ตาม keyword, คืนค่า x offset ที่ใช้ไป ---
int drawIcon(int x, int y, String msg) {
  int ty = y - THAI_ASCENT;  // แปลง baseline → top-left

  if (msg.indexOf("[UP]") >= 0 || msg.indexOf("[Up]") >= 0 || msg.indexOf("[ONLINE]") >= 0) {
    tft.fillCircle(x+5, ty+5+5, 5, TFT_GREEN);
    return 16;
  }
  if (msg.indexOf("[DOWN]") >= 0 || msg.indexOf("[Down]") >= 0) {
    tft.fillCircle(x+5, ty+5+5, 5, TFT_RED);
    return 16;
  }
  if (msg.indexOf("[WARN]") >= 0) {
    // ขยับแกน Y ลงมา +5px ในทุกจุดพิกัด
    tft.fillTriangle(x, ty + 10 + 5, x + 6, ty + 5, x + 12, ty + 10 + 5, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawChar('!', x + 4, ty + 2 + 5, 1);
    return 16;
  }
  if (msg.indexOf("[LOVE]") >= 0 || msg.indexOf("<3") >= 0) {
    tft.fillCircle(x+3, ty+8, 3, TFT_RED);
    tft.fillCircle(x+8, ty+8, 3, TFT_RED);
    tft.fillTriangle(x, ty+9, x+11, ty+9, x+5, ty+16, TFT_RED);
    return 16;
  }
  return 0;
}

// --- วาด 1 บรรทัด feed พร้อม icon ---
void drawFeedLine(int x, int y, String msg, uint16_t color, int font) {
  int offset = drawIcon(x, y, msg);
  String clean = msg;
  clean.replace("[UP]", "");   clean.replace("[DOWN]", "");
  clean.replace("[Up]", "");   clean.replace("[Down]", "");
  clean.replace("[WARN]", ""); clean.replace("[LOVE]", "");
  clean.replace("<3", "");     clean.replace("[ONLINE]", "");
  clean.trim();
  fontHandler.setFont(&NotoSansThai16);
  fontHandler.setDrawColor(color);
  fontHandler.setCursor(x + offset, y - 1);
  fontHandler.print(clean.c_str());
}

// --- push ข้อความใหม่เข้า feed ดันขึ้น ---
void pushFeed(String msg) {
  for (int i = FEED_LINES - 1; i > 0; i--) feed[i] = feed[i-1];
  feed[0] = msg;
}

// --- ฟังก์ชันจัดเลเนต์เอาต์หน้าจอ ---
void refreshUIPanel(String credit, String faceMode) {
  tft.fillScreen(TFT_BLACK);

  // ส่วนที่ 1: แถบหัวระบบด้านบนสุด
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(displayName.c_str(), 15, 12, 1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawCentreString("MaxPlus AI", 160, 12, 1);
  if (wifiConnected) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawRightString(WiFi.localIP().toString().c_str(), 315, 12, 1);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawRightString("NO WIFI", 315, 12, 1);
  }

  // ส่วนที่ 2: ซ้าย - CREDIT REMAINING
  tft.drawRoundRect(12, 25, 175, 60, 8, CYD_PANEL_BLUE);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("CREDIT REMAINING", 22, 33, 1);
  tft.setTextColor(CYD_YELLOW, TFT_BLACK);
  tft.drawString(("$" + credit).c_str(), 22, 47, 4);

  // ส่วนที่ 3: ขวา - โมจิ
  tft.drawRoundRect(197, 25, 110, 60, 15, CYD_PANEL_BLUE);
  if (faceMode == "THINK") {
    tft.fillRect(225, 43, 14, 3, CYD_YELLOW);
    tft.fillRect(265, 43, 14, 3, CYD_YELLOW);
    tft.drawCircle(252, 51, 2, CYD_YELLOW);
    tft.setTextColor(CYD_YELLOW, TFT_BLACK);
    tft.drawCentreString("THINK", 252, 67, 1);
  } else {
    tft.setTextColor(CYD_YELLOW, TFT_BLACK);
    tft.drawString("^", 225, 39, 2);
    tft.drawString("^", 265, 39, 2);
    tft.fillRect(247, 51, 10, 2, CYD_YELLOW);
  }
  tft.fillRect(207, 47, 8, 6, CYD_PINK);
  tft.fillRect(289, 47, 8, 6, CYD_PINK);

  // ส่วนที่ 4: Pager Feed
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawFastHLine(15, 95, 14, TFT_DARKBLUE);
  tft.drawString("[ PAGER ]", 33, 88, 2);
  tft.drawFastHLine(97, 95, 210, TFT_DARKBLUE);
  drawFeedLine(5, 104 + THAI_ASCENT, feed[0], TFT_WHITE, 2);
  for (int i = 1; i < FEED_LINES; i++) {
    drawFeedLine(5, 104 + i*17 + THAI_ASCENT, feed[i], CYD_TEXT_BLUE, 2);
  }
}

// ==================== JSON PARSING (ArduinoJson v7) ====================
float findCredit(JsonVariant doc) {
  if (doc.containsKey("credit_usd")) return doc["credit_usd"].as<float>();
  if (doc.containsKey("user")) {
    JsonVariant user = doc["user"];
    if (user.containsKey("credit_usd")) return user["credit_usd"].as<float>();
  }
  if (doc.containsKey("balance")) return doc["balance"].as<float>();
  if (doc.containsKey("balance_usd")) return doc["balance_usd"].as<float>();
  return -1;
}

// ==================== MQTT ====================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println("MQTT: " + msg);
  pushFeed(msg);
  refreshUIPanel(lastCreditValue, "ONLINE");
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  if (mqtt.connect("CYD-Pager", MQTT_USER, MQTT_PASS)) {
    mqtt.subscribe(MQTT_TOPIC);
    Serial.println("MQTT connected");
  } else {
    Serial.print("MQTT failed: "); Serial.println(mqtt.state());
  }
}

// ==================== WiFi ====================
void connectWiFi() {
  Serial.print("Connecting WiFi to: "); Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while ((WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0,0,0,0)) && attempts < 40) {
    delay(500); Serial.print("."); attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    digitalWrite(RGB_RED, HIGH); digitalWrite(RGB_GREEN, LOW);
    Serial.println("\nWiFi connected!");
  } else {
    wifiConnected = false;
    digitalWrite(RGB_GREEN, HIGH); digitalWrite(RGB_RED, LOW);
    pushFeed("ERROR: CHECK ACCESS POINT");
    Serial.println("\nWiFi connection FAILED");
  }
}

// ==================== API FETCH ====================
void fetchAndDisplay() {
  if (!wifiConnected) return;

  HTTPClient http;
  WiFiClientSecure client;
  client.setCACert(LE_E8_CA);

  http.setTimeout(20000);
  http.setConnectTimeout(10000);
  http.begin(client, API_URL);
  http.addHeader("Authorization", String("Bearer ") + API_KEY);

  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  int httpResponse = http.GET();
  Serial.print("HTTP: "); Serial.println(httpResponse);

  if (httpResponse == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      float credit = findCredit(doc.as<JsonVariant>());
      if (credit >= 0) {
        lastCreditValue = String(credit, 2);
        lastRefresh = millis();
        if (doc["user"]["display_name"].as<String>() != "null")
          displayName = doc["user"]["display_name"].as<String>();
        refreshUIPanel(lastCreditValue, "ONLINE");
      } else {
        pushFeed("Error: credit_usd not found");
        refreshUIPanel("N/A", "THINK");
      }
    } else {
      pushFeed("Error: invalid JSON");
      refreshUIPanel("JSON Err", "THINK");
    }
  } else {
    pushFeed("HTTP ERR: " + String(httpResponse));
    refreshUIPanel("HTTP Err", "THINK");
  }
  http.end();
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  
  // เริ่มต้นระบบหน้าจอ TFT_eSPI
  tft.init();
  tft.setRotation(1);
  
  // ปิดไฟสถานะหลังจอตอนเริ่มบูต
  pinMode(RGB_RED, OUTPUT);   digitalWrite(RGB_RED, HIGH);
  pinMode(RGB_GREEN, OUTPUT); digitalWrite(RGB_GREEN, HIGH);
  pinMode(RGB_BLUE, OUTPUT);  digitalWrite(RGB_BLUE, HIGH);
  
  refreshUIPanel(lastCreditValue, "THINK");
  
  connectWiFi();
  if (wifiConnected) {
    connectMQTT();
    fetchAndDisplay();
  } else {
    refreshUIPanel("ERR", "THINK");
  }
}

// ==================== LOOP ====================
void loop() {
  unsigned long now = millis();

  // MQTT: loop + reconnect อัตโนมัติ
  if (wifiConnected) {
    if (!mqtt.connected()) connectMQTT();
    mqtt.loop();
  }

  if (!wifiConnected && (now - wifiReconnect > 300000)) {
    wifiReconnect = now;
    connectWiFi();
    if (wifiConnected) { connectMQTT(); fetchAndDisplay(); }
  }

  if (now - lastRefresh >= REFRESH_INTERVAL_MS) {
    lastRefresh = now;
    if (wifiConnected) {
      fetchAndDisplay();
    } else {
      pushFeed("OFFLINE - Reconnecting...");
      refreshUIPanel(lastCreditValue, "THINK");
    }
  }
  delay(50);
}