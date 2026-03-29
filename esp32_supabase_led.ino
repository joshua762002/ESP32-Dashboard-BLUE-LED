#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID     = "Joshua-2.4G";
const char* WIFI_PASSWORD = "joshua762002";

const char* SUPABASE_URL = "https://vszeihhsgzmqadnrwckq.supabase.co";
const char* SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InZzemVpaGhzZ3ptcWFkbnJ3Y2txIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzQ0MzM0OTYsImV4cCI6MjA5MDAwOTQ5Nn0.w3u5N-uRh6YZeLddDYRM1WJ2Ij-ds_Y-tDTBvlzPn40";

#define LED_PIN        2
#define POLL_INTERVAL  3000

bool lastLedStatus = false;
unsigned long lastPollTime = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);
  Serial.println("ESP32 Supabase LED Controller");
  connectToWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Disconnected! Reconnecting...");
    connectToWiFi();
    return;
  }
  unsigned long now = millis();
  if (now - lastPollTime >= POLL_INTERVAL) {
    lastPollTime = now;
    fetchLedStatus();
  }
}

void connectToWiFi() {
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  delay(500);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] Failed. Will retry...");
  }
}

void fetchLedStatus() {
  HTTPClient http;
  String endpoint = String(SUPABASE_URL) +
    "/rest/v1/device_control?id=eq.1&select=led_status";
  http.begin(endpoint);
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
  http.addHeader("Content-Type", "application/json");
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.print("[Supabase] Response: ");
    Serial.println(payload);
    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      bool status = doc[0]["led_status"].as<bool>();
      if (status != lastLedStatus) {
        lastLedStatus = status;
        digitalWrite(LED_PIN, status ? HIGH : LOW);
        Serial.println(status ? "[LED] ON" : "[LED] OFF");
      } else {
        Serial.println("[LED] No change.");
      }
    }
  } else {
    Serial.print("[HTTP] Error code: ");
    Serial.println(code);
  }
  http.end();
}