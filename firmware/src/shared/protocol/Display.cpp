#include <ArduinoJson.h>
#include <shared/protocol/Display.h>

namespace domisen::protocol {

bool decodeDisplayCommand(const std::string& payload, std::string& text, std::string& error) {
  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    error = "không phải JSON hợp lệ";
    return false;
  }
  if (!doc.is<JsonObject>()) {
    error = "lệnh phải là một đối tượng JSON";
    return false;
  }
  if (!doc["text"].is<std::string>()) {
    error = "text phải là chuỗi";
    return false;
  }
  text = doc["text"].as<std::string>();
  return true;
}

std::string encodeDisplayState(const std::string& text) {
  JsonDocument doc;
  doc["text"] = text;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
