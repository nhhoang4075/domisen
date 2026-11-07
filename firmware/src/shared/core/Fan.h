// Luật thuần của quạt chỉnh tốc độ. Dùng chung trạng thái bật tắt và mức với đèn (Light.h).
#pragma once
#include <shared/core/Light.h>

namespace domisen::core {

// Quạt nhỏ không quay khi duty thấp, nên mức 1..100 đi tuyến tính từ minDuty đến 1,0 (không bù gamma như đèn).
// Tắt là duty 0.
inline float fanDuty(const LightState& state, float minDuty) {
  if (!state.on) return 0.0f;
  return minDuty + (1.0f - minDuty) * static_cast<float>(state.level) / kMaxLightLevel;
}

}  // namespace domisen::core
