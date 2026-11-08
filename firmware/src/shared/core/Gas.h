// Luật báo động gas có trễ, thuần: không biết phần cứng, không biết thời gian.
#pragma once
#include <shared/core/Hysteresis.h>

namespace domisen::core {

// previous là trạng thái alarm trước đó. Chưa báo động: bật khi raw >= onRaw.
// Đang báo động: chỉ tắt khi raw < offRaw, ở giữa thì giữ nguyên (không nhấp nháy quanh ngưỡng). Cần offRaw < onRaw.
inline bool gasAlarm(bool previous, int raw, int onRaw, int offRaw) {
  return hysteresisAbove(previous, raw, onRaw, offRaw);
}

}  // namespace domisen::core
