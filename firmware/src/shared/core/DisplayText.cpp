#include <algorithm>
#include <iterator>
#include <shared/core/DisplayText.h>

namespace domisen::core {

namespace {

constexpr size_t kMaxLines = 4;
constexpr size_t kMaxLineChars = 16;

// Chữ cái tiếng Việt dạng dựng sẵn (NFC) ngoài ASCII: a ă â e ê i o ô ơ u ư y và đ, chữ thường và chữ hoa, đủ năm dấu thanh.
// Xếp tăng dần để tìm nhị phân. Danh sách này khớp với phông u8g2_font_unifont_t_vietnamese2 (xem docs/mqtt-topics.md).
constexpr char32_t kVietnameseLetters[] = {
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C8, 0x00C9, 0x00CA, 0x00CC, 0x00CD, 0x00D2, 0x00D3, 0x00D4, 0x00D5,
    0x00D9, 0x00DA, 0x00DD, 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E8, 0x00E9, 0x00EA, 0x00EC, 0x00ED, 0x00F2,
    0x00F3, 0x00F4, 0x00F5, 0x00F9, 0x00FA, 0x00FD, 0x0102, 0x0103, 0x0110, 0x0111, 0x0128, 0x0129, 0x0168,
    0x0169, 0x01A0, 0x01A1, 0x01AF, 0x01B0, 0x1EA0, 0x1EA1, 0x1EA2, 0x1EA3, 0x1EA4, 0x1EA5, 0x1EA6, 0x1EA7,
    0x1EA8, 0x1EA9, 0x1EAA, 0x1EAB, 0x1EAC, 0x1EAD, 0x1EAE, 0x1EAF, 0x1EB0, 0x1EB1, 0x1EB2, 0x1EB3, 0x1EB4,
    0x1EB5, 0x1EB6, 0x1EB7, 0x1EB8, 0x1EB9, 0x1EBA, 0x1EBB, 0x1EBC, 0x1EBD, 0x1EBE, 0x1EBF, 0x1EC0, 0x1EC1,
    0x1EC2, 0x1EC3, 0x1EC4, 0x1EC5, 0x1EC6, 0x1EC7, 0x1EC8, 0x1EC9, 0x1ECA, 0x1ECB, 0x1ECC, 0x1ECD, 0x1ECE,
    0x1ECF, 0x1ED0, 0x1ED1, 0x1ED2, 0x1ED3, 0x1ED4, 0x1ED5, 0x1ED6, 0x1ED7, 0x1ED8, 0x1ED9, 0x1EDA, 0x1EDB,
    0x1EDC, 0x1EDD, 0x1EDE, 0x1EDF, 0x1EE0, 0x1EE1, 0x1EE2, 0x1EE3, 0x1EE4, 0x1EE5, 0x1EE6, 0x1EE7, 0x1EE8,
    0x1EE9, 0x1EEA, 0x1EEB, 0x1EEC, 0x1EED, 0x1EEE, 0x1EEF, 0x1EF0, 0x1EF1, 0x1EF2, 0x1EF3, 0x1EF4, 0x1EF5,
    0x1EF6, 0x1EF7, 0x1EF8, 0x1EF9,
};

bool isDisplayable(char32_t codePoint) {
  if (codePoint >= 0x20 && codePoint <= 0x7E) return true;
  return std::binary_search(std::begin(kVietnameseLetters), std::end(kVietnameseLetters), codePoint);
}

// Giải mã một ký tự UTF-8 ở vị trí pos. Trả số byte đã dùng, hoặc 0 nếu chuỗi byte sai (thiếu byte, mã quá dài, ký tự 4 byte).
size_t decodeUtf8(const std::string& text, size_t pos, char32_t& codePoint) {
  const unsigned char lead = static_cast<unsigned char>(text[pos]);
  if (lead < 0x80) {
    codePoint = lead;
    return 1;
  }
  size_t length = 0;
  char32_t value = 0;
  if (lead >= 0xC2 && lead <= 0xDF) {
    length = 2;
    value = lead & 0x1F;
  } else if (lead >= 0xE0 && lead <= 0xEF) {
    length = 3;
    value = lead & 0x0F;
  } else {
    return 0;
  }
  if (pos + length > text.size()) return 0;
  for (size_t index = 1; index < length; ++index) {
    const unsigned char next = static_cast<unsigned char>(text[pos + index]);
    if ((next & 0xC0) != 0x80) return 0;
    value = (value << 6) | (next & 0x3F);
  }
  if (length == 3 && value < 0x800) return 0;
  codePoint = value;
  return length;
}

}  // namespace

bool splitDisplayText(const std::string& text, std::vector<std::string>& lines, std::string& error) {
  lines.clear();
  error.clear();
  if (text.empty()) return true;

  std::string line;
  size_t lineChars = 0;
  size_t pos = 0;
  while (pos < text.size()) {
    char32_t codePoint = 0;
    const size_t used = decodeUtf8(text, pos, codePoint);
    if (used == 0 || (codePoint != '\n' && !isDisplayable(codePoint))) {
      lines.clear();
      error = "text chỉ nhận ASCII in được và chữ cái tiếng Việt (UTF-8 dựng sẵn)";
      return false;
    }
    if (codePoint == '\n') {
      lines.push_back(line);
      line.clear();
      lineChars = 0;
      if (lines.size() >= kMaxLines) {
        lines.clear();
        error = "text vượt quá 4 dòng";
        return false;
      }
    } else {
      line.append(text, pos, used);
      if (++lineChars > kMaxLineChars) {
        lines.clear();
        error = "mỗi dòng tối đa 16 ký tự";
        return false;
      }
    }
    pos += used;
  }

  lines.push_back(line);
  return true;
}

}  // namespace domisen::core
