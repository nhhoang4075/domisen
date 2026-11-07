// Đèn màu WS2812: cả thanh một màu, chỉnh được màu và độ sáng. Nhận PixelStrip qua constructor.
#pragma once
#include <shared/core/ColorLight.h>
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

namespace domisen::services {

class ColorLight : public Device {
 public:
  ColorLight(std::string id, core::PixelStrip& strip);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  void show();

  std::string id_;
  core::PixelStrip& strip_;
  core::ColorLightState state_;
};

}  // namespace domisen::services
