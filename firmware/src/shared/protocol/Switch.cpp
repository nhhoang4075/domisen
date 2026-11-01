#include <ArduinoJson.h>
#include <shared/protocol/Switch.h>

namespace domisen::protocol {

bool decodeSwitchCommand(const std::string& payload, bool& on, std::string& error) {
  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    error = "không phải JSON hợp lệ";
    return false;
  }
  if (!doc.is<JsonObject>()) {
    error = "lệnh phải là một đối tượng JSON";
    return false;
  }
  if (!doc["on"].is<bool>()) {
    error = "lệnh cần có on là true hoặc false";
    return false;
  }
  on = doc["on"].as<bool>();
  return true;
}

std::string encodeSwitchState(bool on) {
  JsonDocument doc;
  doc["on"] = on;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
