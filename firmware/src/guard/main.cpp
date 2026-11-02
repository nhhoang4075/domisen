// Board guard: cửa, bếp, an ninh. Hiện có còi và quạt hút bếp.
// Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Esp32DigitalOut.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/protocol/Topics.h>
#include <shared/services/Hub.h>
#include <shared/services/SwitchDevice.h>

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

static const bool kAlarmActiveLow = true;       // Trạng thái tắt mặc định đưa chân lên HIGH.
static const bool kExhaustFanActiveLow = true;  // Relay active-low; chỉnh theo module thực tế.
static adapters::esp32::Esp32DigitalOut alarmOutput(27, kAlarmActiveLow);       // GPIO27
static adapters::esp32::Esp32DigitalOut exhaustFanRelay(16, kExhaustFanActiveLow);  // GPIO16
static services::SwitchDevice alarmDevice("house/alarm", alarmOutput);
static services::SwitchDevice exhaustFan("kitchen/exhaust-fan", exhaustFanRelay);

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  alarmOutput.begin();
  exhaustFanRelay.begin();
  hub.add(alarmDevice);
  hub.add(exhaustFan);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
