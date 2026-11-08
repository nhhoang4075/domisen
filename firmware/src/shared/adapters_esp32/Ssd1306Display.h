#pragma once
#include <shared/core/Ports.h>

#include <U8g2lib.h>

namespace domisen::adapters::esp32 {

// OLED SSD1306 128x64 qua I2C, vẽ chữ tiếng Việt bằng U8g2. Chân và địa chỉ do main.cpp của board truyền vào.
class Ssd1306Display : public core::TextDisplay {
 public:
  Ssd1306Display(int sdaPin, int sclPin, uint8_t address);
  void begin();
  void show(const std::vector<std::string>& lines) override;

 private:
  int sdaPin_;
  int sclPin_;
  uint8_t address_;
  bool ready_ = false;
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C display_;
};

}  // namespace domisen::adapters::esp32
