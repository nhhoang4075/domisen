// Đèn chỉnh độ sáng bằng PWM. Dùng cho đèn trần, đèn ngủ... chỉ khác id và PwmOutput.
#pragma once
#include <core/Light.h>
#include <core/Ports.h>
#include <services/Device.h>

namespace domisen::services {

class DimmableLight : public Device {
 public:
  DimmableLight(std::string id, core::PwmOutput& pwm);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  core::PwmOutput& pwm_;
  core::LightState state_;
};

}  // namespace domisen::services
