#include <shared/protocol/Switch.h>
#include <shared/services/SwitchDevice.h>

namespace domisen::services {

SwitchDevice::SwitchDevice(std::string id, core::DigitalOutput& output) : id_(std::move(id)), output_(output) {}

void SwitchDevice::begin() { output_.write(on_); }

bool SwitchDevice::onCommand(const std::string& payload, std::string& error) {
  bool on;
  if (!protocol::decodeSwitchCommand(payload, on, error)) return false;
  on_ = on;
  output_.write(on_);
  return true;
}

std::string SwitchDevice::stateJson() const { return protocol::encodeSwitchState(on_); }

}  // namespace domisen::services
