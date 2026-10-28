#pragma once
#include <string>

namespace domisen::protocol {

// {"error":"..."}: gửi lên .../error khi thiết bị không hiểu lệnh
std::string encodeError(const std::string& message);

}  // namespace domisen::protocol
