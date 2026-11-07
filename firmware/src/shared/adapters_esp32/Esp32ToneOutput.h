#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// Còi thụ động: bật thì phát xung 50 % ở tần số toneHz bằng LEDC, tắt thì giữ chân ở mức cố định (idleHigh).
// Module còi kích ở mức thấp thì idleHigh = true, để lúc tắt không có dòng chạy liên tục qua còi.
// Mỗi cặp kênh LEDC (0 và 1, 2 và 3, ...) dùng chung một bộ định thời, tức chung tần số: chọn kênh khác cặp với servo.
class Esp32ToneOutput : public core::DigitalOutput {
 public:
  Esp32ToneOutput(int pin, int channel, int toneHz, bool idleHigh);
  void begin();
  void write(bool on) override;

 private:
  void silence();

  int pin_;
  int channel_;
  int toneHz_;
  bool idleHigh_;
  bool sounding_ = false;
};

}  // namespace domisen::adapters::esp32
