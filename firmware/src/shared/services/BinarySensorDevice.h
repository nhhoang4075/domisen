// Cảm biến số hai trạng thái, chỉ đọc và có chống nhiễu theo thời gian.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <optional>

namespace domisen::services {

class BinarySensorDevice : public Device {
 public:
  BinarySensorDevice(std::string id, std::string key, core::DigitalInput& input,
                     bool activeLow, uint32_t debounceMs = 50);

  const std::string& id() const override { return id_; }
  bool tick(uint32_t nowMs) override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  std::string key_;
  core::DigitalInput& input_;
  bool activeLow_;
  uint32_t debounceMs_;
  bool initialized_ = false;
  bool state_ = false;
  bool candidate_ = false;
  uint32_t candidateSinceMs_ = 0;
};

}  // namespace domisen::services
