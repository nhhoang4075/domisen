// Đèn chỉnh độ sáng bằng PWM. Dùng cho đèn trần, đèn ngủ... chỉ khác id và PwmOutput.
#pragma once
#include <shared/core/Light.h>
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

namespace domisen::services {

class DimmableLight : public Device {
 public:
  DimmableLight(std::string id, core::PwmOutput& pwm);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;
  // Trả true một lần sau khi setOn() đổi trạng thái, để Hub gửi state mới lên broker.
  bool tick(uint32_t nowMs) override;
  // Vẫn được tick khi mất broker để nhận cờ "đã đổi" (Hub không gửi lúc đó, nối lại thì gửi state hiện tại).
  bool runsOffline() const override { return true; }

  bool isOn() const { return state_.on; }
  // Bật hoặc tắt từ code trên board (nút cảm ứng), giống lệnh {"on":...}: bật dùng lại mức sáng đã nhớ, tắt giữ mức cũ.
  void setOn(bool on);

 private:
  std::string id_;
  core::PwmOutput& pwm_;
  core::LightState state_;
  bool changedByBoard_ = false;
};

}  // namespace domisen::services
