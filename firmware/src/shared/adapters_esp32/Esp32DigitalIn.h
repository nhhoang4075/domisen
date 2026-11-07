#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

class Esp32DigitalIn : public core::DigitalInput {
 public:
  explicit Esp32DigitalIn(int pin);
  void begin();
  bool read() override;

 private:
  int pin_;
};

}  // namespace domisen::adapters::esp32
