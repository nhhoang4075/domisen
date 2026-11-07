#include <ArduinoJson.h>
#include <shared/protocol/Position.h>

#include <algorithm>

namespace domisen::protocol {

bool decodePositionCommand(const std::string& payload, const std::vector<std::string>& names, std::string& position,
                           std::string& error) {
  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    error = "không phải JSON hợp lệ";
    return false;
  }
  if (!doc.is<JsonObject>()) {
    error = "lệnh phải là một đối tượng JSON";
    return false;
  }
  // Ba lỗi (thiếu, sai kiểu, sai tên) dùng chung một thông báo nêu đủ danh sách hợp lệ.
  std::string valid;
  for (size_t i = 0; i < names.size(); i++) {
    if (i > 0) valid += ", ";
    valid += names[i];
  }
  const std::string invalid = "position phải là một trong: " + valid;
  if (doc["position"].isNull() || !doc["position"].is<const char*>()) {
    error = invalid;
    return false;
  }
  std::string name = doc["position"].as<const char*>();
  if (std::find(names.begin(), names.end(), name) == names.end()) {
    error = invalid;
    return false;
  }
  position = name;
  return true;
}

std::string encodePositionState(const std::string& position) {
  JsonDocument doc;
  doc["position"] = position;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
