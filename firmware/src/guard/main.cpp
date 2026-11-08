// Board guard: cửa, bếp, an ninh. Hiện có servo cửa chính, đầu đọc thẻ RC522, còi, quạt hút bếp, màn hình OLED và cảm biến gas; các thiết bị khác làm sau.
// Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Esp32AnalogIn.h>
#include <shared/adapters_esp32/Esp32DigitalOut.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/Esp32ToneOutput.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/adapters_esp32/Rc522Reader.h>
#include <shared/adapters_esp32/Ssd1306Display.h>
#include <shared/protocol/Topics.h>
#include <shared/services/CardReaderDevice.h>
#include <shared/services/DisplayDevice.h>
#include <shared/services/GasSensorDevice.h>
#include <shared/services/Hub.h>
#include <shared/services/PositionDevice.h>
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

// Servo cửa SG90 (GPIO4, kênh LEDC 2). Góc là hằng số, chỉnh khi lắp cơ khí.
static adapters::esp32::Esp32Pwm doorPwm(4, 2, 50, 16);
static const float kDoorClosedDeg = 0.0f;
static const float kDoorOpenDeg = 90.0f;
static services::PositionDevice door("entrance/door", doorPwm, {{"closed", kDoorClosedDeg}, {"open", kDoorOpenDeg}});
static adapters::esp32::Rc522Reader rc522(16, 19, 17, 18, 5);  // SS GPIO16, RST GPIO19, SCK GPIO17, MISO GPIO18, MOSI GPIO5 (SPI qua GPIO matrix)
static services::CardReaderDevice cardReader("entrance/card-reader", rc522);

// Còi là module buzzer thụ động 5V: chỉ kêu khi nhận xung, nên dùng Esp32ToneOutput (xung 50 % ở kAlarmToneHz).
// Chân I/O kéo lên VCC (5 V), kích ở mức thấp: lúc tắt thả chân về đầu vào để module tự kéo lên 5 V (xuất 3,3 V chưa đủ tắt hẳn, còi nóng).
// Kênh LEDC 4 khác cặp với servo (kênh 2): mỗi cặp kênh chung bộ định thời nên chung tần số.
static const int kAlarmToneHz = 3000;
static adapters::esp32::Esp32ToneOutput alarmOutput(27, 4, kAlarmToneHz);  // GPIO27, kênh LEDC 4

// Relay quạt hút. false: hút khi chân lên mức cao (như relay quạt phòng khách); module chạy mức thấp thì đổi thành true.
static const bool kExhaustFanActiveLow = false;
static adapters::esp32::Esp32DigitalOut exhaustFanRelay(32, kExhaustFanActiveLow);  // GPIO32
static services::SwitchDevice alarmDevice("house/alarm", alarmOutput);
static services::SwitchDevice exhaustFan("kitchen/exhaust-fan", exhaustFanRelay);

// Cảm biến gas MQ-2: AO qua hai điện trở 10 kΩ (cầu phân áp) vào GPIO35. Xem docs/mqtt-topics.md, mục "Cảm biến gas MQ-2".
// Ngưỡng báo động đặt theo số đo ngày 08/11/2026: không khí sạch khoảng 10 đến 30, bật lửa đang cháy đưa trước cảm biến khoảng 60 đến 90.
// Số đo lúc mới bật nguồn (khoảng 3 phút); chủ dự án đo lại sau khi cảm biến nóng ổn định và khi có gas thật (bật lửa gas chưa châm) rồi chỉnh. Phải có kGasAlarmOffRaw < kGasAlarmOnRaw.
static const int kGasAlarmOnRaw = 50;
static const int kGasAlarmOffRaw = 40;
static const uint32_t kGasWarmupMs = 60000;
static const uint32_t kGasSampleMs = 1000;
static const uint32_t kGasPublishMs = 5000;
static const int kGasSamples = 8;
static adapters::esp32::Esp32AnalogIn gasInput(35);  // GPIO35, ADC1
static services::GasSensorDevice gas("kitchen/gas", gasInput, clock_,
                                     {.onRaw = kGasAlarmOnRaw,
                                      .offRaw = kGasAlarmOffRaw,
                                      .warmupMs = kGasWarmupMs,
                                      .sampleMs = kGasSampleMs,
                                      .publishMs = kGasPublishMs,
                                      .samples = kGasSamples});

static adapters::esp32::Ssd1306Display oled(21, 22, 0x3C);  // I2C: SDA GPIO21, SCL GPIO22, địa chỉ 0x3C
static services::DisplayDevice display("entrance/display", oled);

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  doorPwm.begin();
  hub.add(door);
  rc522.begin();
  hub.add(cardReader);
  alarmOutput.begin();
  exhaustFanRelay.begin();
  gasInput.begin();
  hub.add(alarmDevice);
  hub.add(exhaustFan);
  hub.add(gas);
  oled.begin();
  hub.add(display);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
