// Mã hóa lệnh và trạng thái của thiết bị bật tắt (quạt...).
#pragma once
#include <string>

namespace domisen::protocol {

// Trả false và điền error nếu payload không phải {"on":true} hoặc {"on":false}.
bool decodeSwitchCommand(const std::string& payload, bool& on, std::string& error);
std::string encodeSwitchState(bool on);

}  // namespace domisen::protocol
