#include <shared/adapters_esp32/Ws2812Strip.h>

namespace domisen::adapters::esp32 {

Ws2812Strip::Ws2812Strip(int pin, int numLeds) : pixels_(numLeds, pin, NEO_GRB + NEO_KHZ800) {}

void Ws2812Strip::begin() {
  pixels_.begin();
  pixels_.clear();  // tắt hết trước khi hub chạy
  pixels_.show();
}

void Ws2812Strip::setColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels_.fill(pixels_.Color(r, g, b));
  pixels_.show();
}

}  // namespace domisen::adapters::esp32
