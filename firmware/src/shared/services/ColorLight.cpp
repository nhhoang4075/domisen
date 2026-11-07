#include <shared/protocol/ColorLight.h>
#include <shared/services/ColorLight.h>

namespace domisen::services {

ColorLight::ColorLight(std::string id, core::PixelStrip& strip) : id_(std::move(id)), strip_(strip) {}

void ColorLight::begin() {
  state_.on = false;  // khởi động luôn tắt
  show();
}

bool ColorLight::onCommand(const std::string& payload, std::string& error) {
  protocol::ColorLightCommand cmd;
  if (!protocol::decodeColorLightCommand(payload, cmd, error)) return false;
  state_ = core::applyColorLightCommand(state_, cmd.on, cmd.color, cmd.level);
  show();
  return true;
}

std::string ColorLight::stateJson() const {
  return protocol::encodeColorLightState(state_.on, state_.color, state_.level);
}

void ColorLight::show() {
  core::Rgb out = core::colorLightOutput(state_);
  strip_.setColor(out.r, out.g, out.b);
}

}  // namespace domisen::services
