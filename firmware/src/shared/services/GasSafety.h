// Quy tắc an toàn khi có gas, chạy ngay trên board, không chờ server (docs/mqtt-topics.md, mục "Còi và quạt hút").
// alarm bật: còi và quạt hút bật, quạt hút bị khóa (lệnh tắt từ server bị từ chối). Còi vẫn tắt tiếng được bằng lệnh tắt
// và không tự bật lại cho đến lần báo động sau. alarm tắt: bỏ khóa, tắt cả hai.
#pragma once
#include <shared/services/GasSensorDevice.h>
#include <shared/services/SwitchDevice.h>

namespace domisen::services {

class GasSafety {
 public:
  GasSafety(GasSensorDevice& gas, SwitchDevice& buzzer, SwitchDevice& exhaustFan);
  // Gọi một lần ở setup(), sau khi các thiết bị đã khởi tạo.
  void begin();

 private:
  void onAlarmChange(bool alarm);

  GasSensorDevice& gas_;
  SwitchDevice& buzzer_;
  SwitchDevice& exhaustFan_;
};

}  // namespace domisen::services
