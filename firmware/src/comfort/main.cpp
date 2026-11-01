// Board comfort: phòng khách, phòng ngủ, ban công. Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Dht22Sensor.h>
#include <shared/adapters_esp32/Esp32DigitalOut.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/protocol/Topics.h>
#include <shared/services/ClimateDevice.h>
#include <shared/services/DimmableLight.h>
#include <shared/services/Hub.h>
#include <shared/services/PositionDevice.h>
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

// Servo SG90, mỗi servo một kênh LEDC riêng (0 và 1 đã dùng cho đèn). Góc là hằng số, chỉnh khi lắp cơ khí.
static adapters::esp32::Esp32Pwm curtainPwm(13, 2, 50, 16);     // GPIO13, kênh LEDC 2
static adapters::esp32::Esp32Pwm clotheslinePwm(33, 3, 50, 16);  // GPIO33, kênh LEDC 3
static const float kCurtainOpenDeg = 0.0f;
static const float kCurtainClosedDeg = 90.0f;
static const float kClotheslineInDeg = 0.0f;
static const float kClotheslineOutDeg = 90.0f;
static services::PositionDevice curtain("living/curtain", curtainPwm,
                                        {{"open", kCurtainOpenDeg}, {"closed", kCurtainClosedDeg}});
static services::PositionDevice clothesline("balcony/clothesline", clotheslinePwm,
                                            {{"in", kClotheslineInDeg}, {"out", kClotheslineOutDeg}});

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  ceilingPwm.begin();
  nightPwm.begin();
  curtainPwm.begin();
  clotheslinePwm.begin();
  fanRelay.begin();
  dht.begin();
  hub.add(ceilingLight);
  hub.add(nightLight);
  hub.add(fan);
  hub.add(climate);
  hub.add(curtain);
  hub.add(clothesline);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
