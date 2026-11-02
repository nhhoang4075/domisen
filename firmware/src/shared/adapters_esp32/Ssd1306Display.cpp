#include <Arduino.h>
#include <Wire.h>
#include <shared/adapters_esp32/Ssd1306Display.h>

namespace domisen::adapters::esp32 {

static constexpr int kSdaPin = 21;
static constexpr int kSclPin = 22;
static constexpr uint8_t kAddress = 0x3C;

Ssd1306Display::Ssd1306Display() : display_(128, 64, &Wire, -1) {}

void Ssd1306Display::begin() {
  Wire.begin(kSdaPin, kSclPin);
  display_.begin(SSD1306_SWITCHCAPVCC, kAddress);
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
    display_.setCursor(0, static_cast<int16_t>(index * 16));
    display_.println(lines[index].c_str());
  }
  display_.display();
}

}  // namespace domisen::adapters::esp32
