// Board comfort: phòng khách, phòng ngủ, ban công. Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Dht22Sensor.h>
#include <shared/adapters_esp32/Esp32DigitalIn.h>
#include <shared/adapters_esp32/Esp32DigitalOut.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/protocol/Topics.h>
#include <shared/services/ClimateDevice.h>
#include <shared/services/BinarySensorDevice.h>
#include <shared/services/DimmableLight.h>
#include <shared/services/Hub.h>
#include <shared/services/SwitchDevice.h>

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

static const bool kFanRelayActiveLow = false;  // relay của bạn hút khi IN lên 5V (mức cao); module khác có thể hút ở mức thấp
static adapters::esp32::Esp32DigitalOut fanRelay(16, kFanRelayActiveLow);  // GPIO16
static services::SwitchDevice fan("living/fan", fanRelay);

static adapters::esp32::Dht22Sensor dht(14);  // GPIO14
static services::ClimateDevice climate("living/climate", dht);

static const bool kMotionSensorActiveLow = false;
static const bool kRainSensorActiveLow = true;
static const bool kWallButtonActiveLow = false;
static adapters::esp32::Esp32DigitalIn motionInput(39);    // GPIO39
static adapters::esp32::Esp32DigitalIn rainInput(34);      // GPIO34
static adapters::esp32::Esp32DigitalIn wallButtonInput(27);  // GPIO27
static services::BinarySensorDevice motion("living/motion", "motion", motionInput,
                                           kMotionSensorActiveLow);
static services::BinarySensorDevice rain("balcony/rain", "raining", rainInput,
                                         kRainSensorActiveLow);
static services::BinarySensorDevice wallButton("living/wall-button", "pressed", wallButtonInput,
                                               kWallButtonActiveLow);

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  ceilingPwm.begin();
  nightPwm.begin();
  fanRelay.begin();
  dht.begin();
  motionInput.begin();
  rainInput.begin();
  wallButtonInput.begin();
  hub.add(ceilingLight);
  hub.add(nightLight);
  hub.add(fan);
  hub.add(climate);
  hub.add(motion);
  hub.add(rain);
  hub.add(wallButton);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
