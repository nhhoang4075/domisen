// Mã hóa lệnh và trạng thái của đèn màu: {"on":bool,"color":"#RRGGBB","level":0..100}.
#pragma once
#include <optional>
#include <string>

#include <shared/core/ColorLight.h>

namespace domisen::protocol {

struct ColorLightCommand {
  std::optional<bool> on;
  std::optional<core::Rgb> color;
  std::optional<int> level;
};

// Đọc chuỗi đúng dạng #RRGGBB (chữ hoa hoặc thường). Trả false nếu sai định dạng.
bool parseHexColor(const std::string& text, core::Rgb& out);
// Ghi thành #RRGGBB, chữ hoa.
std::string formatHexColor(const core::Rgb& color);

// Trả false và điền error nếu payload không phải lệnh hợp lệ.
bool decodeColorLightCommand(const std::string& payload, ColorLightCommand& out, std::string& error);
std::string encodeColorLightState(bool on, const core::Rgb& color, int level);

}  // namespace domisen::protocol
