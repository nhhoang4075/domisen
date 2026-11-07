#include <ArduinoJson.h>
#include <shared/protocol/ColorLight.h>

namespace domisen::protocol {

namespace {

// Trả -1 nếu không phải chữ số hex.
int hexDigit(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

}  // namespace

bool parseHexColor(const std::string& text, core::Rgb& out) {
  if (text.size() != 7 || text[0] != '#') return false;
  int v[6];
  for (int i = 0; i < 6; i++) {
    v[i] = hexDigit(text[i + 1]);
    if (v[i] < 0) return false;
  }
  out.r = static_cast<uint8_t>(v[0] * 16 + v[1]);
  out.g = static_cast<uint8_t>(v[2] * 16 + v[3]);
  out.b = static_cast<uint8_t>(v[4] * 16 + v[5]);
  return true;
}

std::string formatHexColor(const core::Rgb& color) {
  static const char* digits = "0123456789ABCDEF";
  std::string out = "#";
  for (uint8_t channel : {color.r, color.g, color.b}) {
    out += digits[channel >> 4];
    out += digits[channel & 0x0F];
  }
  return out;
}

bool decodeColorLightCommand(const std::string& payload, ColorLightCommand& out, std::string& error) {
  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    error = "không phải JSON hợp lệ";
    return false;
  }
  if (!doc.is<JsonObject>()) {
    error = "lệnh phải là một đối tượng JSON";
    return false;
  }
  // Giải mã vào bản tạm, đủ hợp lệ rồi mới ghi ra out
  ColorLightCommand cmd;
  if (!doc["on"].isNull()) {
    if (!doc["on"].is<bool>()) {
      error = "on phải là true hoặc false";
      return false;
    }
    cmd.on = doc["on"].as<bool>();
  }
  if (!doc["color"].isNull()) {
    core::Rgb color;
    if (!doc["color"].is<const char*>() || !parseHexColor(doc["color"].as<std::string>(), color)) {
      error = "color phải là chuỗi #RRGGBB";
      return false;
    }
    cmd.color = color;
  }
  if (!doc["level"].isNull()) {
    if (!doc["level"].is<int>()) {
      error = "level phải là số nguyên từ 0 đến 100";
      return false;
    }
    cmd.level = doc["level"].as<int>();
  }
  if (!cmd.on && !cmd.color && !cmd.level) {
    error = "lệnh cần có on, color hoặc level";
    return false;
  }
  out = cmd;
  return true;
}

std::string encodeColorLightState(bool on, const core::Rgb& color, int level) {
  JsonDocument doc;
  doc["on"] = on;
  doc["color"] = formatHexColor(color);
  doc["level"] = level;
  std::string out;
  serializeJson(doc, out);
  return out;
}

}  // namespace domisen::protocol
