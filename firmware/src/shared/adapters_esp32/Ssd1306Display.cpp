#include <Arduino.h>
#include <Wire.h>
#include <shared/adapters_esp32/Ssd1306Display.h>

namespace domisen::adapters::esp32 {

// Chữ cỡ 1 cao 8 px; mỗi dòng cách nhau 16 px để chừa một hàng trống cho dễ đọc, 4 dòng vừa đủ 64 px.
static constexpr int kLineStridePx = 16;

Ssd1306Display::Ssd1306Display(int sdaPin, int sclPin, uint8_t address)
    : sdaPin_(sdaPin), sclPin_(sclPin), address_(address), display_(128, 64, &Wire, -1) {}

void Ssd1306Display::begin() {
  Wire.begin(sdaPin_, sclPin_);
  if (!display_.begin(SSD1306_SWITCHCAPVCC, address_)) {
    Serial.printf("OLED không phản hồi ở địa chỉ 0x%02X (SDA GPIO%d, SCL GPIO%d), kiểm tra dây I2C\n", address_, sdaPin_,
                  sclPin_);
    return;
  }
  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.display();
}

void Ssd1306Display::show(const std::vector<std::string>& lines) {
  display_.clearDisplay();
  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  for (size_t index = 0; index < lines.size(); ++index) {
    display_.setCursor(0, static_cast<int16_t>(index * kLineStridePx));
    display_.println(lines[index].c_str());
  }
  display_.display();
}

}  // namespace domisen::adapters::esp32
