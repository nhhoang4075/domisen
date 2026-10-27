#include "ceiling_light.h"

#include <ArduinoJson.h>
#include <Domisen.h>
#include <math.h>

namespace ceiling_light {

const char* CMD_TOPIC = "domisen/living/ceiling-light/cmd";
static const char* STATE_TOPIC = "domisen/living/ceiling-light/state";

static const int PIN = 25;
static const int CHANNEL = 0;         // kênh PWM của ESP32
static const int FREQ = 5000;         // Hz, đủ cao để mắt không thấy nháy
static const int RESOLUTION = 10;     // bit: 0..1023
static const int MAX_DUTY = (1 << RESOLUTION) - 1;

static bool isOn = false;
static int level = 100;               // 1..100, nhớ lại khi tắt rồi bật

// Mắt cảm nhận độ sáng không tuyến tính: bù gamma để kéo 0..100 thấy đều.
static void apply() {
  int duty = 0;
  if (isOn) duty = static_cast<int>(pow(level / 100.0, 2.2) * MAX_DUTY + 0.5);
  ledcWrite(CHANNEL, duty);
}

void begin() {
  ledcSetup(CHANNEL, FREQ, RESOLUTION);
  ledcAttachPin(PIN, CHANNEL);
  apply();
}

void publishState() {
  JsonDocument doc;
  doc["on"] = isOn;
  doc["level"] = level;
  String out;
  serializeJson(doc, out);
  domisen::publish(STATE_TOPIC, out, true);  // retain: ai mới đăng ký cũng thấy trạng thái hiện tại
}

// Lệnh: {"on":true}, {"on":false}, {"level":40}, hoặc {"on":true,"level":80}.
// level = 0 nghĩa là tắt (giữ nguyên mức sáng cũ để lần bật sau dùng lại).
void onCommand(const String& payload) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("đèn trần: lệnh không phải JSON hợp lệ (%s)\n", err.c_str());
    return;
  }
  if (doc["level"].is<int>()) {
    int v = constrain(doc["level"].as<int>(), 0, 100);
    if (v == 0) {
      isOn = false;
    } else {
      level = v;
      if (!doc["on"].is<bool>()) isOn = true;  // chỉ gửi level thì hiểu là bật
    }
  }
  if (doc["on"].is<bool>()) isOn = doc["on"].as<bool>();
  apply();
  Serial.printf("đèn trần: on=%d level=%d\n", isOn, level);
  publishState();
}

}  // namespace ceiling_light
