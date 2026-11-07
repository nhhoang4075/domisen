// Board guard: cửa, bếp, an ninh. Hiện có servo cửa chính. Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/protocol/Topics.h>
#include <shared/services/Hub.h>
#include <shared/services/PositionDevice.h>

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

// Servo cửa SG90 (GPIO4, kênh LEDC 2). Góc là hằng số, chỉnh khi lắp cơ khí.
static adapters::esp32::Esp32Pwm doorPwm(4, 2, 50, 16);
static const float kDoorClosedDeg = 0.0f;
static const float kDoorOpenDeg = 90.0f;
static services::PositionDevice door("entrance/door", doorPwm, {{"closed", kDoorClosedDeg}, {"open", kDoorOpenDeg}});

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  doorPwm.begin();
  hub.add(door);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
