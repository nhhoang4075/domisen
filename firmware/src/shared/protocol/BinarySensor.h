// Mã hóa trạng thái cảm biến nhị phân.
#pragma once
#include <string>

namespace domisen::protocol {

std::string encodeBinarySensorState(const std::string& key, bool value);
std::string encodeBinarySensorUnknown(const std::string& key);

}  // namespace domisen::protocol
