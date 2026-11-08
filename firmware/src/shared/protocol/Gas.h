// Mã hóa trạng thái cảm biến gas.
#pragma once
#include <string>

namespace domisen::protocol {

// {"ppm_raw":1830,"alarm":true}
std::string encodeGasState(int ppmRaw, bool alarm);
// Đang làm nóng, chưa đọc: {"ppm_raw":null,"alarm":false}
std::string encodeGasWarmup();

}  // namespace domisen::protocol
