#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// Đọc ADC1 12 bit, suy hao 11 dB (đọc được tới gần 3,3 V). Chỉ dùng chân ADC1 (GPIO32 đến 39) để đọc được khi WiFi bật.
class Esp32AnalogIn : public core::AnalogInput {
 public:
  explicit Esp32AnalogIn(int pin);
  void begin();
  uint16_t read() override;

 private:
  int pin_;
};

}  // namespace domisen::adapters::esp32
