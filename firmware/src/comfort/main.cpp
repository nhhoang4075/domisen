// Board comfort: phòng khách, phòng ngủ, ban công. Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Dht22Sensor.h>
#include <shared/adapters_esp32/Esp32AnalogIn.h>
#include <shared/adapters_esp32/Esp32DigitalIn.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/adapters_esp32/Ws2812Strip.h>
#include <shared/core/Fan.h>
#include <shared/protocol/Topics.h>
#include <shared/services/BinarySensorDevice.h>
#include <shared/services/ClimateDevice.h>
#include <shared/services/ColorLight.h>
#include <shared/services/DimmableLight.h>
#include <shared/services/Hub.h>
#include <shared/services/LightSensorDevice.h>
#include <shared/services/PositionDevice.h>
#include <shared/services/SpeedFan.h>
#include <shared/services/TouchToggle.h>

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
static const float kFanMinDuty = 0.3f;     // duty thấp nhất mà quạt đang quay vẫn giữ được vòng quay (ứng với level 1); khởi động đã có kick-start
static const uint32_t kFanKickMs = 500;   // kick-start (chạy hết ga lúc khởi động): chạy hết ga bấy nhiêu ms khi đi từ đứng yên lên duty thấp
static adapters::esp32::Esp32Pwm fanPwm(16, 4, kFanPwmHz);  // GPIO16, kênh LEDC 4
static services::SpeedFan fan("living/fan", fanPwm, clock_, kFanMinDuty, kFanKickMs);

static adapters::esp32::Dht22Sensor dht(14);  // GPIO14
static services::ClimateDevice climate("living/climate", dht);

// Mức tín hiệu "bật" của từng cảm biến (chủ dự án chỉnh khi thử trên module thật):
static const bool kMotionSensorActiveLow = false;  // PIR: HIGH là có chuyển động
static const bool kRainSensorActiveLow = true;     // mưa (chân DO): LOW là đang mưa, module kéo DO xuống khi ướt
static const bool kWallButtonActiveLow = false;    // TTP223: HIGH là đang chạm
// Cảm biến mưa dùng chống nhiễu dài hơn (mặc định của PIR và nút là 50 ms): chip so sánh LM393 trên module không có
// độ trễ nên DO nhấp nháy khi nhỏ nước, và mưa là hiện tượng chậm. Chủ dự án quyết định 07/11/2026.
static const uint32_t kRainDebounceMs = 1000;
static adapters::esp32::Esp32DigitalIn motionInput(39);      // GPIO39
static adapters::esp32::Esp32DigitalIn rainInput(35);        // GPIO35
static adapters::esp32::Esp32DigitalIn wallButtonInput(27);  // GPIO27
static services::BinarySensorDevice motion("living/motion", "motion", motionInput,
                                           kMotionSensorActiveLow);
static services::BinarySensorDevice rain("balcony/rain", "raining", rainInput,
                                         kRainSensorActiveLow, kRainDebounceMs);
static services::BinarySensorDevice wallButton("living/wall-button", "pressed", wallButtonInput,
                                               kWallButtonActiveLow);

// Chạm nút thì đèn trần bật hoặc tắt ngay trên board, kể cả khi mất mạng. Xem docs/mqtt-topics.md, mục "Nút cảm ứng điều khiển đèn trần".
static services::TouchToggle wallTouch(wallButton, ceilingLight);

// Cảm biến ánh sáng: module quang trở LDR có LM393, chân AO vào GPIO34 (ADC1), cấp 3,3V, chân DO không dùng.
// Xem docs/mqtt-topics.md, mục "Cảm biến ánh sáng". Ba hằng số dưới đây là giá trị tạm: chủ dự án đo trên module thật
// (che tay thì AO tăng hay giảm; số ở phòng sáng, tối, chạng vạng) rồi chỉnh. light_raw luôn là số lớn nghĩa là sáng hơn.
static const bool kLightAoFallsWhenBright = true;  // true nếu AO giảm khi sáng hơn
static const int kLightDarkBelowRaw = 800;          // dark bật khi light_raw dưới số này
static const int kLightBrightAtOrAboveRaw = 1200;   // dark tắt khi light_raw đạt số này (phải >= kLightDarkBelowRaw)
static adapters::esp32::Esp32AnalogIn lightInput(34);  // GPIO34, ADC1
static services::LightSensorDevice lightSensor("living/light-sensor", lightInput,
                                               {.darkBelowRaw = kLightDarkBelowRaw,
                                                .brightAtOrAboveRaw = kLightBrightAtOrAboveRaw,
                                                .aoFallsWhenBright = kLightAoFallsWhenBright});

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
  motionInput.begin();
  rainInput.begin();
  wallButtonInput.begin();
  lightInput.begin();
  moodStrip.begin();
  hub.add(ceilingLight);
  hub.add(nightLight);
  hub.add(fan);
  hub.add(climate);
  hub.add(motion);
  hub.add(rain);
  hub.add(wallButton);
  wallTouch.begin();
  hub.add(lightSensor);
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
