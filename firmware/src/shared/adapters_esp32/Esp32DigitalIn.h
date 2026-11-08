#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

class Esp32DigitalIn : public core::DigitalInput {
 public:
  // pullup = true: dùng điện trở kéo lên nội (INPUT_PULLUP), chỉ cho chân có pull-up nội (không phải GPIO34 đến 39).
  explicit Esp32DigitalIn(int pin, bool pullup = false);
  void begin();
  bool read() override;

 private:
  int pin_;
  bool pullup_;
};

}  // namespace domisen::adapters::esp32
