// Chạm nút cảm ứng thì đổi bật tắt đèn ngay trên board, không chờ server (docs/mqtt-topics.md, mục
// "Nút cảm ứng điều khiển đèn trần"). Chỉ phản ứng khi bắt đầu chạm (giá trị đổi sang true sau chống nhiễu):
// giữ tay không lặp lại, nhả tay không làm gì, và lần đọc đầu sau khi khởi động không tính là chạm.
#pragma once
#include <shared/services/BinarySensorDevice.h>
#include <shared/services/DimmableLight.h>

namespace domisen::services {

class TouchToggle {
 public:
  TouchToggle(BinarySensorDevice& button, DimmableLight& light);
  // Gọi một lần ở setup(). Cho nút chạy cả khi mất kết nối broker.
  void begin();

 private:
  BinarySensorDevice& button_;
  DimmableLight& light_;
};

}  // namespace domisen::services
