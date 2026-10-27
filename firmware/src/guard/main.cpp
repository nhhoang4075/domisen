// Board guard: cửa, bếp, an ninh. Chưa có thiết bị nào, mới chỉ nối broker và báo trạng thái.
// Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <adapters_esp32/ArduinoClock.h>
#include <adapters_esp32/MqttTransport.h>
#include <protocol/Topics.h>
#include <services/Hub.h>

#include "domisen_config.h"

using namespace domisen;

static const char* NODE = "guard";

static adapters::esp32::MqttTransport transport({
    .clientId = NODE,
    .hostname = std::string("domisen-") + NODE,
    .wifiSsid = WIFI_SSID,
    .wifiPass = WIFI_PASS,
    .host = MQTT_HOST,
    .port = MQTT_PORT,
    .user = MQTT_USER,
    .pass = MQTT_PASS,
    .willTopic = protocol::nodeStatusTopic(NODE),
});
static adapters::esp32::ArduinoClock clock_;
static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
