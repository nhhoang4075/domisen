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

// Kick-start (chạy hết ga lúc khởi động): quạt đứng yên cần duty cao hơn lúc đã quay (ma sát tĩnh lớn hơn ma sát động). Khi đi từ đứng yên
// (duty đang ra là 0) lên một duty thấp hơn 1,0 thì phải chạy hết ga một lúc trước, rồi mới hạ về duty đã đặt.
inline bool fanNeedsKick(float currentDuty, float targetDuty) {
  return currentDuty <= 0.0f && targetDuty > 0.0f && targetDuty < 1.0f;
}

}  // namespace domisen::core
