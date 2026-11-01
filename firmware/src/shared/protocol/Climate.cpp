#include <ArduinoJson.h>
#include <shared/protocol/Climate.h>

#include <cmath>

namespace domisen::protocol {

static float roundTenth(float v) { return std::round(v * 10.0f) / 10.0f; }

std::string encodeClimateState(float temp, float humidity) {
  JsonDocument doc;
  doc["temp"] = roundTenth(temp);
  doc["humidity"] = roundTenth(humidity);
  std::string out;
  serializeJson(doc, out);
  return out;
}

std::string encodeClimateUnavailable() {
  JsonDocument doc;
  doc["temp"] = nullptr;
  doc["humidity"] = nullptr;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
