// Luật của cảm biến ánh sáng, thuần: chuẩn hóa chiều của AO và cờ "trời tối" có trễ.
#pragma once
#include <shared/core/Hysteresis.h>

namespace domisen::core {

constexpr int kAdcMax = 4095;

// Chuẩn hóa để số lớn nghĩa là sáng hơn, không phụ thuộc chiều của AO. aoFallsWhenBright: AO giảm khi sáng hơn.
inline int normalizeLight(int raw, bool aoFallsWhenBright) {
  const int clamped = raw < 0 ? 0 : (raw > kAdcMax ? kAdcMax : raw);
  return aoFallsWhenBright ? kAdcMax - clamped : clamped;
}

// previous là cờ dark trước đó. Chưa tối: bật khi lightRaw < darkBelow. Đang tối: chỉ tắt khi lightRaw >= brightAtOrAbove,
// ở giữa thì giữ nguyên. Cần brightAtOrAbove >= darkBelow.
inline bool lightIsDark(bool previous, int lightRaw, int darkBelow, int brightAtOrAbove) {
  return hysteresisBelow(previous, lightRaw, darkBelow, brightAtOrAbove);
}

}  // namespace domisen::core
