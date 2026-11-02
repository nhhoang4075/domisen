// Mã hóa và giải mã lệnh cùng trạng thái màn hình.
#pragma once
#include <string>

namespace domisen::protocol {

bool decodeDisplayCommand(const std::string& payload, std::string& text, std::string& error);
std::string encodeDisplayState(const std::string& text);

}  // namespace domisen::protocol
