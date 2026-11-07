#include <shared/protocol/BinarySensor.h>
#include <shared/services/BinarySensorDevice.h>

namespace domisen::services {

BinarySensorDevice::BinarySensorDevice(std::string id, std::string key, core::DigitalInput& input,
                                       bool activeLow, uint32_t debounceMs)
    : id_(std::move(id)),
      key_(std::move(key)),
      input_(input),
      activeLow_(activeLow),
      debounceMs_(debounceMs) {}

bool BinarySensorDevice::tick(uint32_t nowMs) {
  const bool reading = input_.read() != activeLow_;
  if (!initialized_) {
    initialized_ = true;
    state_ = reading;
    candidate_ = reading;
    candidateSinceMs_ = nowMs;
    return true;
  }

  if (reading != candidate_) {
    candidate_ = reading;
    candidateSinceMs_ = nowMs;
  }
  if (candidate_ == state_ || nowMs - candidateSinceMs_ < debounceMs_) return false;

  state_ = candidate_;
  return true;
}

bool BinarySensorDevice::onCommand(const std::string&, std::string& error) {
  error = "cảm biến chỉ đọc, không nhận lệnh";
  return false;
}

std::string BinarySensorDevice::stateJson() const {
  if (!initialized_) return protocol::encodeBinarySensorUnknown(key_);
  return protocol::encodeBinarySensorState(key_, state_);
}

}  // namespace domisen::services
