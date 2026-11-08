// Cảm biến số hai trạng thái, chỉ đọc và có chống nhiễu theo thời gian.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <cstdint>
#include <functional>
#include <string>

namespace domisen::services {

class BinarySensorDevice : public Device {
 public:
  BinarySensorDevice(std::string id, std::string key, core::DigitalInput& input,
                     bool activeLow, uint32_t debounceMs = 50);

  const std::string& id() const override { return id_; }
  bool tick(uint32_t nowMs) override;
  // Mặc định false. Bật cho nút điều khiển đèn trên board: vẫn được tick khi mất kết nối broker (Device::runsOffline).
  bool runsOffline() const override { return runsOffline_; }
  void setRunsOffline(bool value) { runsOffline_ = value; }
  // Gọi mỗi khi giá trị (đã qua chống nhiễu) đổi. Lần đọc đầu sau khi khởi động không gọi, vì đó chưa phải một thay đổi.
  void onChange(std::function<void(bool value)> handler) { changeHandler_ = std::move(handler); }
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  std::string key_;
  core::DigitalInput& input_;
  bool activeLow_;
  uint32_t debounceMs_;
  bool runsOffline_ = false;
  std::function<void(bool)> changeHandler_;
  bool initialized_ = false;
  bool state_ = false;
  bool candidate_ = false;
  uint32_t candidateSinceMs_ = 0;
};

}  // namespace domisen::services
