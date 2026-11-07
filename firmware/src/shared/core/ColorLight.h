// Nguyên lý của đèn màu (thanh WS2812, cả thanh một màu). Không biết JSON, MQTT hay chân GPIO.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>

#include <shared/core/Light.h>

namespace domisen::core {

struct Rgb {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  bool operator==(const Rgb& other) const { return r == other.r && g == other.g && b == other.b; }
};

// Trắng ấm #FFB46B
constexpr Rgb kDefaultColor{255, 180, 107};

struct ColorLightState {
  bool on = false;
  Rgb color = kDefaultColor;   // nhớ lại khi tắt rồi bật
  int level = kMaxLightLevel;  // 1..100, nhớ lại khi tắt rồi bật
};

// on, color, level đều tùy chọn (ít nhất một cái do protocol kiểm tra):
//   color     : luôn được lưu, và bật nếu lệnh không nói rõ on
//   level > 0 : đặt mức, và bật nếu lệnh không nói rõ on
//   level = 0 : tắt, giữ mức cũ
//   on        : nếu có thì quyết định cuối cùng
inline ColorLightState applyColorLightCommand(ColorLightState state, std::optional<bool> on,
                                              std::optional<Rgb> color, std::optional<int> level) {
  if (color) {
    state.color = *color;
    if (!on) state.on = true;
  }
  if (level) {
    int v = std::clamp(*level, 0, kMaxLightLevel);
    if (v == 0) {
      state.on = false;
    } else {
      state.level = v;
      if (!on) state.on = true;
    }
  }
  if (on) state.on = *on;
  return state;
}

// Màu thực tế ra thanh LED: tắt thì (0,0,0); bật thì mỗi kênh nhân level/100, làm tròn, không bù gamma.
inline Rgb colorLightOutput(const ColorLightState& state) {
  if (!state.on) return Rgb{};
  auto scale = [&](uint8_t channel) {
    return static_cast<uint8_t>((channel * state.level + kMaxLightLevel / 2) / kMaxLightLevel);
  };
  return Rgb{scale(state.color.r), scale(state.color.g), scale(state.color.b)};
}

}  // namespace domisen::core
