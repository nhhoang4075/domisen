// Mã hóa lệnh và trạng thái của đèn chỉnh độ sáng.
#pragma once
#include <optional>
#include <string>

namespace domisen::protocol {

struct LightCommand {
  std::optional<bool> on;
  std::optional<int> level;
};

// Trả false và điền error nếu payload không phải lệnh hợp lệ.
bool decodeLightCommand(const std::string& payload, LightCommand& out, std::string& error);
std::string encodeLightState(bool on, int level);

}  // namespace domisen::protocol
