// Nguyên lý của đèn chỉnh được độ sáng. Không biết JSON, MQTT hay chân GPIO.
#pragma once
#include <algorithm>
#include <cmath>
#include <optional>

namespace domisen::core {

constexpr int kMaxLightLevel = 100;
constexpr float kLightGamma = 2.2f;

struct LightState {
  bool on = false;
  int level = kMaxLightLevel;  // 1..100, nhớ lại khi tắt rồi bật
};

// on và level đều tùy chọn:
//   level > 0 : đặt mức, và bật nếu lệnh không nói rõ on
//   level = 0 : tắt, giữ mức cũ
//   on        : nếu có thì quyết định cuối cùng
inline LightState applyLightCommand(LightState state, std::optional<bool> on, std::optional<int> level) {
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

// Mắt cảm nhận độ sáng không tuyến tính, nên bù gamma để kéo 0..100 thấy đều.
inline float lightDuty(const LightState& state) {
  if (!state.on) return 0.0f;
  return std::pow(static_cast<float>(state.level) / kMaxLightLevel, kLightGamma);
}

}  // namespace domisen::core
