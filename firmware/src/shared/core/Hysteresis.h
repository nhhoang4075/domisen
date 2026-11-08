// Cờ có trễ tổng quát, thuần: không biết phần cứng, không biết thời gian.
// Hai ngưỡng khác nhau để giá trị dao động quanh một mức không làm cờ nhấp nháy.
#pragma once

namespace domisen::core {

// Cờ bật khi value cao. Đang tắt: bật khi value >= onAtOrAbove. Đang bật: chỉ tắt khi value < offBelow, ở giữa giữ nguyên.
// Cần offBelow <= onAtOrAbove.
inline bool hysteresisAbove(bool previous, int value, int onAtOrAbove, int offBelow) {
  if (previous) return value >= offBelow;
  return value >= onAtOrAbove;
}

// Cờ bật khi value thấp. Đang tắt: bật khi value < onBelow. Đang bật: chỉ tắt khi value >= offAtOrAbove, ở giữa giữ nguyên.
// Cần offAtOrAbove >= onBelow.
inline bool hysteresisBelow(bool previous, int value, int onBelow, int offAtOrAbove) {
  if (previous) return value < offAtOrAbove;
  return value < onBelow;
}

}  // namespace domisen::core
