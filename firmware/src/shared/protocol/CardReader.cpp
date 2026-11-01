#include <ArduinoJson.h>
#include <shared/protocol/CardReader.h>

namespace domisen::protocol {

std::string encodeCardReaderState(const std::string& uid) {
  JsonDocument doc;
  doc["uid"] = uid;
  std::string out;
  serializeJson(doc, out);
  return out;
}

std::string encodeCardReaderEmpty() {
  JsonDocument doc;
  doc["uid"] = nullptr;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
