// Luật thuần cho văn bản có thể hiển thị trên OLED.
#pragma once
#include <string>
#include <vector>

namespace domisen::core {

bool splitDisplayText(const std::string& text, std::vector<std::string>& lines, std::string& error);

}  // namespace domisen::core
