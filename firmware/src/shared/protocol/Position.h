// Mã hóa lệnh và trạng thái của servo hai vị trí có tên: {"position":"open"}.
#pragma once
#include <string>
#include <vector>

namespace domisen::protocol {

// names là các vị trí hợp lệ của thiết bị, theo thứ tự khai báo. Trả false và điền error nếu payload sai.
// Thiếu trường, sai kiểu hay tên không có trong danh sách đều báo lỗi nêu đủ danh sách hợp lệ.
bool decodePositionCommand(const std::string& payload, const std::vector<std::string>& names, std::string& position,
                           std::string& error);
std::string encodePositionState(const std::string& position);

}  // namespace domisen::protocol
