// Mã hóa trạng thái cảm biến ánh sáng.
#pragma once
#include <string>

namespace domisen::protocol {

// {"light_raw":2310,"dark":false}
std::string encodeLightSensorState(int lightRaw, bool dark);
// Chưa đọc lần nào: {"light_raw":null,"dark":false}
std::string encodeLightSensorUnavailable();

}  // namespace domisen::protocol
