#pragma once
#include <domisen/core/Ports.h>

namespace domisen::adapters::fake {

class FakeClock : public core::Clock {
 public:
  uint32_t nowMs() const override { return now_; }
  void advance(uint32_t ms) { now_ += ms; }

 private:
  uint32_t now_ = 0;
};

}  // namespace domisen::adapters::fake
