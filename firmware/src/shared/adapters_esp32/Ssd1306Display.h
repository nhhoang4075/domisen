#pragma once
#include <shared/core/Ports.h>

#include <Adafruit_SSD1306.h>

namespace domisen::adapters::esp32 {

// OLED SSD1306 128x64 qua I2C. Chân và địa chỉ do main.cpp của board truyền vào.
class Ssd1306Display : public core::TextDisplay {
 public:
  Ssd1306Display(int sdaPin, int sclPin, uint8_t address);
  void begin();
  void show(const std::vector<std::string>& lines) override;

 private:
  int sdaPin_;
  int sclPin_;
  uint8_t address_;
  Adafruit_SSD1306 display_;
};

}  // namespace domisen::adapters::esp32
