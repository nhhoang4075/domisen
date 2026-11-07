#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// Còi thụ động: bật thì phát xung 50 % ở tần số toneHz bằng LEDC, tắt thì thả chân về đầu vào (trở kháng cao).
// Module còi có điện trở kéo I/O lên VCC (5 V) và kích ở mức thấp. Chân ESP32 xuất mức cao chỉ có 3,3 V, thấp hơn
// 5 V nên transistor trong module vẫn hé bật, còi nóng. Thả chân thì điện trở kéo lên đưa I/O lên đủ 5 V, transistor tắt hẳn.
// Mỗi cặp kênh LEDC (0 và 1, 2 và 3, ...) dùng chung một bộ định thời, tức chung tần số: chọn kênh khác cặp với servo.
class Esp32ToneOutput : public core::DigitalOutput {
 public:
  Esp32ToneOutput(int pin, int channel, int toneHz);
  void begin();
  void write(bool on) override;

 private:
  void silence();

  int pin_;
  int channel_;
  int toneHz_;
  bool sounding_ = false;
};

}  // namespace domisen::adapters::esp32
