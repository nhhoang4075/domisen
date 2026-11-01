#include <shared/protocol/Climate.h>
#include <shared/services/ClimateDevice.h>

namespace domisen::services {

ClimateDevice::ClimateDevice(std::string id, core::ClimateSensor& sensor, uint32_t intervalMs)
    : id_(std::move(id)), sensor_(sensor), intervalMs_(intervalMs) {}

bool ClimateDevice::tick(uint32_t nowMs) {
  if (lastReadMs_ && nowMs - *lastReadMs_ < intervalMs_) return false;
  lastReadMs_ = nowMs;
  reading_ = sensor_.read();  // lỗi thì reading_ rỗng, state báo null chứ không giữ số cũ
  return true;
}

bool ClimateDevice::onCommand(const std::string&, std::string& error) {
  error = "cảm biến chỉ đọc, không nhận lệnh";
  return false;
}

std::string ClimateDevice::stateJson() const {
  if (!reading_) return protocol::encodeClimateUnavailable();
  return protocol::encodeClimateState(reading_->temp, reading_->humidity);
}

}  // namespace domisen::services
