#include <ArduinoJson.h>
#include <shared/protocol/Light.h>

namespace domisen::protocol {

bool decodeLightCommand(const std::string& payload, LightCommand& out, std::string& error) {
  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    error = "không phải JSON hợp lệ";
    return false;
  }
  if (!doc.is<JsonObject>()) {
    error = "lệnh phải là một đối tượng JSON";
    return false;
  }
  out = LightCommand{};
  if (!doc["on"].isNull()) {
    if (!doc["on"].is<bool>()) {
      error = "on phải là true hoặc false";
      return false;
    }
    out.on = doc["on"].as<bool>();
  }
  if (!doc["level"].isNull()) {
    if (!doc["level"].is<int>()) {
      error = "level phải là số nguyên từ 0 đến 100";
      return false;
    }
    out.level = doc["level"].as<int>();
  }
  if (!out.on && !out.level) {
    error = "lệnh cần có on hoặc level";
    return false;
  }
  return true;
}

std::string encodeLightState(bool on, int level) {
  JsonDocument doc;
  doc["on"] = on;
  doc["level"] = level;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
