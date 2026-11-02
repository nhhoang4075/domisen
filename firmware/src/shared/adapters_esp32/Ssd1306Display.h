#pragma once
#include <shared/core/Ports.h>

#include <Adafruit_SSD1306.h>

namespace domisen::adapters::esp32 {

class Ssd1306Display : public core::TextDisplay {
 public:
  Ssd1306Display();
  void begin();
  void show(const std::vector<std::string>& lines) override;

 private:
  Adafruit_SSD1306 display_;
};

}  // namespace domisen::adapters::esp32
