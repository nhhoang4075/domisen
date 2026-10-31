#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// PWM bằng LEDC của ESP32. Mỗi đầu ra cần một kênh riêng (0..15).
class Esp32Pwm : public core::PwmOutput {
 public:
  Esp32Pwm(int pin, int channel, int freqHz = 5000, int resolutionBits = 10);
  void begin();
  void write(float duty) override;

 private:
  int pin_;
  int channel_;
  int freqHz_;
  int bits_;
};

}  // namespace domisen::adapters::esp32
