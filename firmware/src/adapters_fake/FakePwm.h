#pragma once
#include <core/Ports.h>

#include <algorithm>
#include <vector>

namespace domisen::adapters::fake {

class FakePwm : public core::PwmOutput {
 public:
  void write(float duty) override {
    last = std::clamp(duty, 0.0f, 1.0f);  // giống adapter thật: giới hạn trong 0..1
    history.push_back(last);
  }

  float last = 0.0f;
  std::vector<float> history;
};

}  // namespace domisen::adapters::fake
