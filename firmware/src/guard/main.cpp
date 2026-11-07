// Board guard: cửa, bếp, an ninh. Hiện có servo cửa chính, đầu đọc thẻ RC522, còi và quạt hút bếp; các thiết bị khác làm sau.
// Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/Esp32DigitalOut.h>
#include <shared/adapters_esp32/Esp32Pwm.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/adapters_esp32/Rc522Reader.h>
#include <shared/protocol/Topics.h>
#include <shared/services/CardReaderDevice.h>
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

// Còi (module buzzer 5V) và relay quạt hút. false: bật khi chân lên mức cao (như relay quạt phòng khách);
// module chạy mức thấp thì đổi thành true. Chủ dự án chỉnh khi thử trên board.
static const bool kAlarmActiveLow = false;
static const bool kExhaustFanActiveLow = false;
static adapters::esp32::Esp32DigitalOut alarmOutput(27, kAlarmActiveLow);              // GPIO27
static adapters::esp32::Esp32DigitalOut exhaustFanRelay(32, kExhaustFanActiveLow);     // GPIO32
static services::SwitchDevice alarmDevice("house/alarm", alarmOutput);
static services::SwitchDevice exhaustFan("kitchen/exhaust-fan", exhaustFanRelay);

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  doorPwm.begin();
  hub.add(door);
  rc522.begin();
  hub.add(cardReader);
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
