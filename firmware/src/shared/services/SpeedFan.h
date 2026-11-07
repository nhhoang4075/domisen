// Quạt chỉnh tốc độ bằng PWM: lệnh on và level 0..100 như đèn (xem docs/mqtt-topics.md).
// Khác đèn ở hai điểm: duty đi tuyến tính từ minDuty (fanDuty), và có "đá khởi động" (fanNeedsKick):
// quạt đứng yên cần duty cao hơn lúc đã quay, nên khi đi từ đứng yên lên duty thấp thì chạy hết ga kickMs trước.
#pragma once
#include <shared/core/Fan.h>
#include <shared/core/Light.h>
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

namespace domisen::services {

class SpeedFan : public Device {
 public:
  SpeedFan(std::string id, core::PwmOutput& pwm, core::Clock& clock, float minDuty, uint32_t kickMs);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool tick(uint32_t nowMs) override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  void apply();
  void write(float duty);

  std::string id_;
  core::PwmOutput& pwm_;
  core::Clock& clock_;
  float minDuty_;
  uint32_t kickMs_;
  core::LightState state_;
  float written_ = 0.0f;   // duty đang ra đầu ra
  bool kicking_ = false;
  uint32_t kickEnd_ = 0;
};

}  // namespace domisen::services
