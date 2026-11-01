// Mã hóa trạng thái của cảm biến nhiệt độ, độ ẩm.
#pragma once
#include <string>

namespace domisen::protocol {

// Làm tròn 1 chữ số thập phân: {"temp":27.5,"humidity":62}
std::string encodeClimateState(float temp, float humidity);
// Cảm biến chưa đo được: {"temp":null,"humidity":null}
std::string encodeClimateUnavailable();

}  // namespace domisen::protocol
