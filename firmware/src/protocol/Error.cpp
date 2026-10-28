#include <ArduinoJson.h>
#include <protocol/Error.h>

namespace domisen::protocol {

std::string encodeError(const std::string& message) {
  JsonDocument doc;
  doc["error"] = message;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
