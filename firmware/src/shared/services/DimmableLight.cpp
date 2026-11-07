#include <shared/protocol/Light.h>
#include <shared/services/DimmableLight.h>

namespace domisen::services {

DimmableLight::DimmableLight(std::string id, core::PwmOutput& pwm, DutyFn duty)
    : id_(std::move(id)), pwm_(pwm), duty_(std::move(duty)) {}

void DimmableLight::begin() { pwm_.write(duty_(state_)); }

bool DimmableLight::onCommand(const std::string& payload, std::string& error) {
  protocol::LightCommand cmd;
  if (!protocol::decodeLightCommand(payload, cmd, error)) return false;
  state_ = core::applyLightCommand(state_, cmd.on, cmd.level);
  pwm_.write(duty_(state_));
  return true;
}

std::string DimmableLight::stateJson() const { return protocol::encodeLightState(state_.on, state_.level); }

}  // namespace domisen::services
