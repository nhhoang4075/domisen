#pragma once
#include <Adafruit_NeoPixel.h>
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// Thanh WS2812 dùng thư viện Adafruit NeoPixel (thứ tự màu GRB, 800 kHz). Cả thanh một màu.
class Ws2812Strip : public core::PixelStrip {
 public:
  Ws2812Strip(int pin, int numLeds);
  void begin();
  void setColor(uint8_t r, uint8_t g, uint8_t b) override;

 private:
  Adafruit_NeoPixel pixels_;
};

}  // namespace domisen::adapters::esp32
