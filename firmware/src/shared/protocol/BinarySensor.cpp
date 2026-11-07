#include <ArduinoJson.h>
#include <shared/protocol/BinarySensor.h>

namespace domisen::protocol {

std::string encodeBinarySensorState(const std::string& key, bool value) {
  JsonDocument doc;
  doc[key] = value;
  std::string out;
  serializeJson(doc, out);
  return out;
}

std::string encodeBinarySensorUnknown(const std::string& key) {
  JsonDocument doc;
  doc[key] = nullptr;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
