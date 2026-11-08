#include <ArduinoJson.h>
#include <shared/protocol/Gas.h>

namespace domisen::protocol {

std::string encodeGasState(int ppmRaw, bool alarm) {
  JsonDocument doc;
  doc["ppm_raw"] = ppmRaw;
  doc["alarm"] = alarm;
  std::string out;
  serializeJson(doc, out);
  return out;
}

std::string encodeGasWarmup() {
  JsonDocument doc;
  doc["ppm_raw"] = nullptr;
  doc["alarm"] = false;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
