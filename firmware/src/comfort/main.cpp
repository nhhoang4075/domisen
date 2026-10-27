// Bản thử đầu tiên: nối WiFi và broker, nháy LED trên board, gửi uptime mỗi 5 giây.
// Gửi bất kỳ tin nào tới domisen/node/comfort/ping để thấy nó in ra Serial.
#include <Arduino.h>
#include <Domisen.h>

static const char* NODE = "comfort";
static const int LED_PIN = 2;  // LED có sẵn trên board ESP32 DevKit
static unsigned long lastBeat = 0;

static void onMessage(const String& topic, const String& payload) {
  Serial.printf("nhận %s = %s\n", topic.c_str(), payload.c_str());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  domisen::begin(NODE, onMessage);
  domisen::subscribe(String("domisen/node/") + NODE + "/ping");
}

void loop() {
  domisen::loop();
  if (millis() - lastBeat >= 5000) {
    lastBeat = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    domisen::publish(String("domisen/node/") + NODE + "/uptime", String(millis() / 1000));
  }
}
