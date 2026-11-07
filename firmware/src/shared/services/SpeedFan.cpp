#include <shared/protocol/Light.h>
#include <shared/services/SpeedFan.h>

namespace domisen::services {

SpeedFan::SpeedFan(std::string id, core::PwmOutput& pwm, core::Clock& clock, float minDuty, uint32_t kickMs)
    : id_(std::move(id)), pwm_(pwm), clock_(clock), minDuty_(minDuty), kickMs_(kickMs) {}

void SpeedFan::begin() {
  kicking_ = false;
  write(0.0f);  // khởi động luôn ở trạng thái tắt, dù đầu ra đang ở đâu
  apply();
}

void SpeedFan::write(float duty) {
  written_ = duty;
  pwm_.write(duty);
}

void SpeedFan::apply() {
  const float target = core::fanDuty(state_, minDuty_);
  if (target <= 0.0f) {
    kicking_ = false;
    write(0.0f);
  } else if (kicking_) {
    write(1.0f);  // đang đá: lệnh mới chỉ đổi mức đích, hết thời gian đá mới hạ xuống
  } else if (core::fanNeedsKick(written_, target)) {
    kicking_ = true;
    kickEnd_ = clock_.nowMs() + kickMs_;
    write(1.0f);
  } else {
    write(target);
  }
}

bool SpeedFan::tick(uint32_t nowMs) {
  // Phép trừ 32 bit vẫn đúng khi đồng hồ quay về 0.
  if (kicking_ && static_cast<int32_t>(nowMs - kickEnd_) >= 0) {
    kicking_ = false;
    write(core::fanDuty(state_, minDuty_));
  }
  return false;  // đá khởi động không đổi state, không cần gửi lại
}

bool SpeedFan::onCommand(const std::string& payload, std::string& error) {
  protocol::LightCommand cmd;
  if (!protocol::decodeLightCommand(payload, cmd, error)) return false;
  state_ = core::applyLightCommand(state_, cmd.on, cmd.level);
  apply();
  return true;
}

std::string SpeedFan::stateJson() const { return protocol::encodeLightState(state_.on, state_.level); }

}  // namespace domisen::services
