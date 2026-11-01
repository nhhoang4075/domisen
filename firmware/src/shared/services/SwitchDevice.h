// Thiết bị chỉ có bật và tắt (quạt...). Chỉ khác nhau id và DigitalOutput.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

namespace domisen::services {

class SwitchDevice : public Device {
 public:
  SwitchDevice(std::string id, core::DigitalOutput& output);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  core::DigitalOutput& output_;
  bool on_ = false;
};

}  // namespace domisen::services
