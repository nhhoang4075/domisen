#pragma once
#include <Arduino.h>
#include <domisen/core/Ports.h>

namespace domisen::adapters::esp32 {

class ArduinoClock : public core::Clock {
 public:
  uint32_t nowMs() const override { return millis(); }
};

}  // namespace domisen::adapters::esp32
