// Board guard: cửa, bếp, an ninh. Hiện có đầu đọc thẻ RC522; các thiết bị khác làm sau.
// Chỉ lắp ráp: adapter, thiết bị, hub. Xem docs/architecture.md.
#include <Arduino.h>
#include <shared/adapters_esp32/ArduinoClock.h>
#include <shared/adapters_esp32/MqttTransport.h>
#include <shared/adapters_esp32/Rc522Reader.h>
#include <shared/protocol/Topics.h>
#include <shared/services/CardReaderDevice.h>
#include <shared/services/Hub.h>

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
static adapters::esp32::Rc522Reader rc522(5, 4, 18, 19, 23);  // SS GPIO5, RST GPIO4, SCK GPIO18, MISO GPIO19, MOSI GPIO23
static services::CardReaderDevice cardReader("entrance/card-reader", rc522);

static services::Hub hub(NODE, transport, clock_);

void setup() {
  Serial.begin(115200);
  rc522.begin();
  hub.add(cardReader);
  hub.begin();
  transport.begin();
}

void loop() {
  transport.loop();
  hub.loop();
}
