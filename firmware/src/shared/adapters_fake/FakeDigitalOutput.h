#pragma once
#include <shared/core/Ports.h>

#include <vector>

namespace domisen::adapters::fake {

class FakeDigitalOutput : public core::DigitalOutput {
 public:
  void write(bool on) override {
    last = on;
    history.push_back(on);
  }

  bool last = false;
  std::vector<bool> history;
};

}  // namespace domisen::adapters::fake
