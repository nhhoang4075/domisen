#include <shared/core/Servo.h>
#include <shared/protocol/Position.h>
#include <shared/services/PositionDevice.h>

namespace domisen::services {

PositionDevice::PositionDevice(std::string id, core::PwmOutput& pwm, std::vector<PositionOption> positions)
    : id_(std::move(id)), pwm_(pwm), positions_(std::move(positions)) {}

void PositionDevice::begin() {
  if (!positions_.empty()) moveTo(0);
}

bool PositionDevice::onCommand(const std::string& payload, std::string& error) {
  std::vector<std::string> names;
  names.reserve(positions_.size());
  for (const PositionOption& p : positions_) names.push_back(p.name);
  std::string position;
  if (!protocol::decodePositionCommand(payload, names, position, error)) return false;
  for (std::size_t i = 0; i < positions_.size(); i++) {
    if (positions_[i].name == position) {
      moveTo(i);
      return true;
    }
  }
  error = "position không hợp lệ";  // không tới đây: decode đã kiểm tra tên
  return false;
}

std::string PositionDevice::stateJson() const {
  if (positions_.empty()) return "{}";
  return protocol::encodePositionState(positions_[current_].name);
}

void PositionDevice::moveTo(std::size_t index) {
  current_ = index;
  pwm_.write(core::servoDuty(positions_[index].angle));
}

}  // namespace domisen::services
