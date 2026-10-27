// Board comfort: phòng khách, phòng ngủ, ban công. Xem docs/gpio.md.
#include <Arduino.h>
#include <Domisen.h>

#include "ceiling_light.h"

static const char* NODE = "comfort";
static const int LED_PIN = 2;  // LED có sẵn trên board, nháy theo nhịp gửi uptime
static unsigned long lastBeat = 0;

static void onMessage(const String& topic, const String& payload) {
  Serial.printf("nhận %s = %s\n", topic.c_str(), payload.c_str());
  if (topic == ceiling_light::CMD_TOPIC) ceiling_light::onCommand(payload);
}

static void publishAllStates() {
  ceiling_light::publishState();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  ceiling_light::begin();
  domisen::begin(NODE, onMessage);
  domisen::onConnect(publishAllStates);
  domisen::subscribe(String("domisen/node/") + NODE + "/ping");
  domisen::subscribe(ceiling_light::CMD_TOPIC);
}

void loop() {
  domisen::loop();
  if (millis() - lastBeat >= 5000) {
    lastBeat = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    domisen::publish(String("domisen/node/") + NODE + "/uptime", String(millis() / 1000));
  }
}
