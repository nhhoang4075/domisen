#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::fake {

class FakeClimateSensor : public core::ClimateSensor {
 public:
  std::optional<core::ClimateReading> read() override {
    reads++;
    return next;
  }

  std::optional<core::ClimateReading> next;  // giá trị lần đọc sau; rỗng là đọc lỗi
  int reads = 0;
};

}  // namespace domisen::adapters::fake
