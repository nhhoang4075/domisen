// Cảm biến nhiệt độ, độ ẩm. Đo theo chu kỳ, Hub gửi kết quả lên state. Không nhận lệnh.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <optional>

namespace domisen::services {

class ClimateDevice : public Device {
 public:
  ClimateDevice(std::string id, core::ClimateSensor& sensor, uint32_t intervalMs = 10000);

  const std::string& id() const override { return id_; }
  bool tick(uint32_t nowMs) override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  core::ClimateSensor& sensor_;
  uint32_t intervalMs_;
  std::optional<uint32_t> lastReadMs_;  // chưa có: đo ngay ở lần tick đầu
  std::optional<core::ClimateReading> reading_;
};

}  // namespace domisen::services
