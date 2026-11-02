#include <shared/core/DisplayText.h>

namespace domisen::core {

bool splitDisplayText(const std::string& text, std::vector<std::string>& lines, std::string& error) {
  lines.clear();
  error.clear();
  if (text.empty()) return true;

  std::string line;
  for (unsigned char character : text) {
    if (character == '\n') {
      lines.push_back(line);
      line.clear();
      if (lines.size() >= 4) {
        lines.clear();
        error = "text vượt quá 4 dòng";
        return false;
      }
      continue;
    }
    if (character < 0x20 || character > 0x7e) {
      lines.clear();
      error = "text chỉ nhận ký tự ASCII in được";
      return false;
    }
    line.push_back(static_cast<char>(character));
    if (line.size() > 21) {
      lines.clear();
      error = "mỗi dòng tối đa 21 ký tự";
      return false;
    }
  }

  lines.push_back(line);
  return true;
}

}  // namespace domisen::core
