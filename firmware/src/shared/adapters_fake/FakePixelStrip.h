#pragma once
#include <shared/core/Ports.h>

#include <vector>

namespace domisen::adapters::fake {

class FakePixelStrip : public core::PixelStrip {
 public:
  struct Color {
    uint8_t r, g, b;
    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b; }
  };

  void setColor(uint8_t r, uint8_t g, uint8_t b) override {
    last = {r, g, b};
    history.push_back(last);
  }

  Color last{0, 0, 0};
  std::vector<Color> history;
};

}  // namespace domisen::adapters::fake
