#include <ArduinoJson.h>
#include <shared/protocol/LightSensor.h>

namespace domisen::protocol {

std::string encodeLightSensorState(int lightRaw, bool dark) {
  JsonDocument doc;
  doc["light_raw"] = lightRaw;
  doc["dark"] = dark;
  std::string out;
  serializeJson(doc, out);
  return out;
}

std::string encodeLightSensorUnavailable() {
  JsonDocument doc;
  doc["light_raw"] = nullptr;
  doc["dark"] = false;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
