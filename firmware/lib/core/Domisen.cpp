#include "Domisen.h"

#include <PubSubClient.h>
#include <WiFi.h>

#include "domisen_config.h"

namespace domisen {

namespace {
WiFiClient net;
PubSubClient mqtt(net);
String node;
String statusTopic;
CommandHandler handler = nullptr;

constexpr size_t MAX_SUBS = 24;
String subs[MAX_SUBS];
size_t subCount = 0;

unsigned long lastTry = 0;
unsigned long backoff = 1000;  // ms, tăng dần tới 15 giây

void onMessage(char* topic, byte* data, unsigned int len) {
  if (!handler) return;
  String payload;
  payload.reserve(len);
  for (unsigned int i = 0; i < len; i++) payload += static_cast<char>(data[i]);
  handler(String(topic), payload);
}

bool connectMqtt() {
  bool ok = (strlen(MQTT_USER) > 0)
                ? mqtt.connect(node.c_str(), MQTT_USER, MQTT_PASS, statusTopic.c_str(), 1, true, "offline")
                : mqtt.connect(node.c_str(), nullptr, nullptr, statusTopic.c_str(), 1, true, "offline");
  if (!ok) return false;
  mqtt.publish(statusTopic.c_str(), "online", true);
  for (size_t i = 0; i < subCount; i++) mqtt.subscribe(subs[i].c_str());
  return true;
}
}  // namespace

void begin(const char* nodeId, CommandHandler onMsg) {
  node = nodeId;
  statusTopic = String("domisen/node/") + node + "/status";
  handler = onMsg;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname((String("domisen-") + node).c_str());  // tên hiện trên router
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void subscribe(const String& topic) {
  if (subCount < MAX_SUBS) subs[subCount++] = topic;
  if (mqtt.connected()) mqtt.subscribe(topic.c_str());
}

bool publish(const String& topic, const String& payload, bool retain) {
  return mqtt.connected() && mqtt.publish(topic.c_str(), payload.c_str(), retain);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected()) {
    backoff = 1000;
    mqtt.loop();
    return;
  }
  if (millis() - lastTry < backoff) return;
  lastTry = millis();
  if (connectMqtt()) {
    Serial.printf("[%s] đã nối broker %s\n", node.c_str(), MQTT_HOST);
  } else {
    backoff = min<unsigned long>(backoff * 2, 15000);
    Serial.printf("[%s] chưa nối được broker (mã %d), thử lại sau %lu ms\n", node.c_str(), mqtt.state(), backoff);
  }
}

}  // namespace domisen
