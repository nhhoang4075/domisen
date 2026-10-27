#include <protocol/Light.h>
#include <services/DimmableLight.h>

namespace domisen::services {

DimmableLight::DimmableLight(std::string id, core::PwmOutput& pwm) : id_(std::move(id)), pwm_(pwm) {}

void DimmableLight::begin() { pwm_.write(core::lightDuty(state_)); }

bool DimmableLight::onCommand(const std::string& payload, std::string& error) {
  protocol::LightCommand cmd;
  if (!protocol::decodeLightCommand(payload, cmd, error)) return false;
  state_ = core::applyLightCommand(state_, cmd.on, cmd.level);
  pwm_.write(core::lightDuty(state_));
  return true;
}

std::string DimmableLight::stateJson() const { return protocol::encodeLightState(state_.on, state_.level); }

}  // namespace domisen::services
