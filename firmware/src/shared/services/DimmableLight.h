// Thiết bị chỉnh mức bằng PWM (lệnh on, level 0..100). Dùng cho đèn trần, đèn ngủ, quạt phòng khách...
// chỉ khác id, PwmOutput và luật đổi trạng thái ra duty (mặc định lightDuty, có bù gamma cho đèn).
#pragma once
#include <functional>

#include <shared/core/Light.h>
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

namespace domisen::services {

class DimmableLight : public Device {
 public:
  using DutyFn = std::function<float(const core::LightState&)>;

  DimmableLight(std::string id, core::PwmOutput& pwm, DutyFn duty = core::lightDuty);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  core::PwmOutput& pwm_;
  DutyFn duty_;
  core::LightState state_;
};

}  // namespace domisen::services
