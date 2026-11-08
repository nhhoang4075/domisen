#include <shared/protocol/Switch.h>
#include <shared/services/SwitchDevice.h>

namespace domisen::services {

SwitchDevice::SwitchDevice(std::string id, core::DigitalOutput& output) : id_(std::move(id)), output_(output) {}

void SwitchDevice::begin() { output_.write(on_); }

bool SwitchDevice::tick(uint32_t) {
  const bool changed = changedByBoard_;
  changedByBoard_ = false;
  return changed;
}

bool SwitchDevice::onCommand(const std::string& payload, std::string& error) {
  bool on;
  if (!protocol::decodeSwitchCommand(payload, on, error)) return false;
  if (locked_ && !on) {
    error = lockReason_;
    return false;
  }
  on_ = on;
  output_.write(on_);
  return true;
}

std::string SwitchDevice::stateJson() const { return protocol::encodeSwitchState(on_); }

void SwitchDevice::setOn(bool on) {
  if (on == on_) return;
  on_ = on;
  output_.write(on_);
  changedByBoard_ = true;
}

void SwitchDevice::lockOn(std::string reason) {
  locked_ = true;
  lockReason_ = std::move(reason);
}

void SwitchDevice::unlock() {
  locked_ = false;
  lockReason_.clear();
}

}  // namespace domisen::services
