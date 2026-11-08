#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::fake {

class FakeAnalogInput : public core::AnalogInput {
 public:
  uint16_t read() override {
    ++reads;
    return value;
  }

  uint16_t value = 0;
  int reads = 0;
};

}  // namespace domisen::adapters::fake
