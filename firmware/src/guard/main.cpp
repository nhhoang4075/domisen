// Board guard: cửa, bếp, an ninh. Hiện có servo cửa chính, đầu đọc thẻ RC522, còi, quạt hút bếp, màn hình OLED và cảm biến gas; các thiết bị khác làm sau.
// Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Esp32AnalogIn.h>
#include <shared/adapters_esp32/Esp32DigitalIn.h>
#include <shared/adapters_esp32/Esp32DigitalOut.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/Esp32ToneOutput.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/adapters_esp32/Rc522Reader.h>
#include <shared/adapters_esp32/Ssd1306Display.h>
#include <shared/protocol/Topics.h>
#include <shared/services/BinarySensorDevice.h>
#include <shared/services/CardReaderDevice.h>
#include <shared/services/DisplayDevice.h>
#include <shared/services/GasSafety.h>
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
// Ngưỡng báo động do chủ dự án chọn ngày 08/11/2026, sau khi đo (mỗi giá trị là một lần gửi state, cách nhau 5 giây):
// không khí sạch khoảng 0 đến 30; bật lửa đang cháy đưa trước cảm biến 60 đến 90;
// gas thật (bật lửa gas bóp van không châm) lên 1485, 942, 530, 421 rồi giảm dần về dưới 50 trong khoảng 30 giây.
// 1200 và 1000 nằm trong vùng ADC đo chính xác (150 đến 3100 mV tại chân), xa hẳn nền sạch (cao hơn khoảng 40 lần).
// Muốn nhạy hơn thì hạ hai số này, muốn ít báo giả hơn thì nâng. Phải có kGasAlarmOffRaw < kGasAlarmOnRaw.
static const int kGasAlarmOnRaw = 1200;
static const int kGasAlarmOffRaw = 1000;
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

// Quy tắc an toàn: có gas thì còi và quạt hút bật ngay trên board (kể cả khi mất mạng), quạt hút bị khóa không tắt được bằng lệnh.
static services::GasSafety gasSafety(gas, alarmDevice, exhaustFan);

// Công tắc từ MC-38 (cửa sổ phòng ngủ): một dây vào GPIO26, một dây vào GND, dùng pull-up nội.
// Nam châm ở gần (cửa sổ đóng) thì tiếp điểm đóng, chân xuống LOW; nam châm xa hoặc dây rút ra thì HIGH, tức báo mở.
// Xem docs/mqtt-topics.md, mục "Cảm biến hai trạng thái".
static const bool kWindowSensorActiveLow = false;  // HIGH là cửa sổ mở (key "open")
static adapters::esp32::Esp32DigitalIn windowInput(26, true);  // GPIO26, INPUT_PULLUP
static services::BinarySensorDevice window("bedroom/window", "open", windowInput, kWindowSensorActiveLow);

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
  windowInput.begin();
  hub.add(alarmDevice);
  hub.add(exhaustFan);
  hub.add(gas);
  hub.add(window);
  gasSafety.begin();
  oled.begin();
  hub.add(display);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
