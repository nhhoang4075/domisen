// Board comfort: phòng khách, phòng ngủ, ban công. Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <adapters_esp32/ArduinoClock.h>
#include <adapters_esp32/Esp32Pwm.h>
#include <adapters_esp32/MqttTransport.h>
#include <protocol/Topics.h>
#include <services/DimmableLight.h>
#include <services/Hub.h>

#include "domisen_config.h"

using namespace domisen;

static const char* NODE = "comfort";

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

static adapters::esp32::Esp32Pwm ceilingPwm(25, 0);  // GPIO25, kênh LEDC 0
static adapters::esp32::Esp32Pwm nightPwm(26, 1);    // GPIO26, kênh LEDC 1

static services::DimmableLight ceilingLight("living/ceiling-light", ceilingPwm);
static services::DimmableLight nightLight("bedroom/night-light", nightPwm);

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  ceilingPwm.begin();
  nightPwm.begin();
  hub.add(ceilingLight);
  hub.add(nightLight);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
