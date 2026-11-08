#include <Arduino.h>
#include <Wire.h>
#include <shared/adapters_esp32/Ssd1306Display.h>

namespace domisen::adapters::esp32 {

// Phông Unifont tiếng Việt cao 16 px, nên mỗi dòng cách nhau 16 px và 4 dòng vừa đủ 64 px; chữ Latin rộng 8 px nên mỗi dòng 16 ký tự.
// Dùng bản vietnamese2 vì bản vietnamese1 thiếu chữ Ư.
static constexpr int kLineStridePx = 16;
static constexpr int kScreenWidthPx = 128;
static constexpr int kScreenHeightPx = 64;

Ssd1306Display::Ssd1306Display(int sdaPin, int sclPin, uint8_t address)
    : sdaPin_(sdaPin),
      sclPin_(sclPin),
      address_(address),
      display_(U8G2_R0, U8X8_PIN_NONE, static_cast<uint8_t>(sclPin), static_cast<uint8_t>(sdaPin)) {}

void Ssd1306Display::begin() {
  // U8g2 không biết OLED có mặt hay không, nên tự hỏi địa chỉ I2C trước để báo lỗi rõ ràng.
  Wire.begin(sdaPin_, sclPin_);
  Wire.beginTransmission(address_);
  if (Wire.endTransmission() != 0) {
    Serial.printf("OLED không phản hồi ở địa chỉ 0x%02X (SDA GPIO%d, SCL GPIO%d), kiểm tra dây I2C\n", address_, sdaPin_,
                  sclPin_);
    return;
  }
  display_.setI2CAddress(static_cast<uint8_t>(address_ << 1));  // U8g2 dùng địa chỉ 8 bit
  display_.begin();
  display_.setFont(u8g2_font_unifont_t_vietnamese2);
  display_.setFontPosTop();
  display_.clearBuffer();
  display_.sendBuffer();
  ready_ = true;
}

// Căn giữa: mỗi dòng căn giữa theo chiều ngang, cả khối chữ căn giữa theo chiều dọc.
void Ssd1306Display::show(const std::vector<std::string>& lines) {
  if (!ready_) return;
  display_.clearBuffer();
  const int blockHeight = static_cast<int>(lines.size()) * kLineStridePx;
  const int top = (kScreenHeightPx - blockHeight) / 2;
  for (size_t index = 0; index < lines.size(); ++index) {
    const int width = display_.getUTF8Width(lines[index].c_str());
    const int left = (kScreenWidthPx - width) / 2;
    display_.drawUTF8(left, top + static_cast<int>(index) * kLineStridePx, lines[index].c_str());
  }
  display_.sendBuffer();
}

}  // namespace domisen::adapters::esp32
