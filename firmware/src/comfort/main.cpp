// Board comfort: phòng khách, phòng ngủ, ban công. Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Dht22Sensor.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/adapters_esp32/Ws2812Strip.h>
#include <shared/core/Fan.h>
#include <shared/protocol/Topics.h>
#include <shared/services/ClimateDevice.h>
#include <shared/services/ColorLight.h>
#include <shared/services/DimmableLight.h>
#include <shared/services/Hub.h>
#include <shared/services/PositionDevice.h>

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

// Quạt phòng khách chỉnh tốc độ: module MOSFET LR7843 có opto EL817 (chậm, nên không dùng 25 kHz), PWM vào GPIO16.
// Kênh LEDC 4 (cặp 4 và 5) tách khỏi đèn (0, 1) và servo (2, 3). Hai hằng số chỉnh khi thử trên quạt thật.
static const int kFanPwmHz = 5000;        // quạt rít thì thử tần số khác
static const float kFanMinDuty = 0.35f;    // duty thấp nhất mà quạt vẫn tự quay (ứng với level 1)
static adapters::esp32::Esp32Pwm fanPwm(16, 4, kFanPwmHz);  // GPIO16, kênh LEDC 4
static services::DimmableLight fan("living/fan", fanPwm,
                                   [](const core::LightState& s) { return core::fanDuty(s, kFanMinDuty); });

static adapters::esp32::Dht22Sensor dht(14);  // GPIO14
static services::ClimateDevice climate("living/climate", dht);

static adapters::esp32::Ws2812Strip moodStrip(33, 8);  // GPIO33, thanh 8 bóng
static services::ColorLight moodLight("living/mood-light", moodStrip);

// Servo SG90, mỗi servo một kênh LEDC riêng (0 và 1 đã dùng cho đèn). Góc là hằng số, chỉnh khi lắp cơ khí.
static adapters::esp32::Esp32Pwm curtainPwm(13, 2, 50, 16);     // GPIO13, kênh LEDC 2
static adapters::esp32::Esp32Pwm clotheslinePwm(32, 3, 50, 16);  // GPIO32, kênh LEDC 3
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
  fanPwm.begin();
  dht.begin();
  moodStrip.begin();
  hub.add(ceilingLight);
  hub.add(nightLight);
  hub.add(fan);
  hub.add(climate);
  hub.add(moodLight);
  hub.add(curtain);
  hub.add(clothesline);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
