#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::fake {

class FakeDigitalInput : public core::DigitalInput {
 public:
  bool read() override { return value; }

  bool value = false;
};

}  // namespace domisen::adapters::fake
