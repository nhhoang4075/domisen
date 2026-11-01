// Servo hai vị trí có tên (cửa, rèm, giàn phơi). Chỉ khác nhau id, PwmOutput và danh sách vị trí.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <cstddef>
#include <string>
#include <vector>

namespace domisen::services {

// Một vị trí có tên và góc (độ). Danh sách không được rỗng, vị trí đầu tiên là vị trí khi khởi động.
struct PositionOption {
  std::string name;
  float angle;
};

class PositionDevice : public Device {
 public:
  PositionDevice(std::string id, core::PwmOutput& pwm, std::vector<PositionOption> positions);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  void moveTo(std::size_t index);

  std::string id_;
  core::PwmOutput& pwm_;
  std::vector<PositionOption> positions_;
  std::size_t current_ = 0;
};

}  // namespace domisen::services
