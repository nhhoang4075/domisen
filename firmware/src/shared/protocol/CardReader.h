// Mã hóa trạng thái của đầu đọc thẻ.
#pragma once
#include <string>

namespace domisen::protocol {

// UID đã là hex chữ hoa không dấu cách: {"uid":"A3F2197C"}
std::string encodeCardReaderState(const std::string& uid);
// Chưa đọc thẻ nào: {"uid":null}
std::string encodeCardReaderEmpty();

}  // namespace domisen::protocol
