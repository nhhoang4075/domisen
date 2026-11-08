#include <algorithm>
#include <shared/core/Gas.h>
#include <shared/protocol/Gas.h>
#include <shared/services/GasSensorDevice.h>

namespace domisen::services {

GasSensorDevice::GasSensorDevice(std::string id, core::AnalogInput& input, core::Clock& clock, Config config)
    : id_(std::move(id)), input_(input), clock_(clock), config_(config) {
  // Ngưỡng tắt cao hơn ngưỡng bật thì mất độ trễ và alarm có thể kẹt bật: ép về bằng ngưỡng bật.
  config_.offRaw = std::min(config_.offRaw, config_.onRaw);
  config_.samples = std::max(config_.samples, 1);
}

void GasSensorDevice::begin() {
  bootMs_ = clock_.nowMs();
  lastSampleMs_.reset();
  raw_.reset();
  alarm_ = false;
}

bool GasSensorDevice::tick(uint32_t nowMs) {
  if (nowMs - bootMs_ < config_.warmupMs) return false;
  if (lastSampleMs_ && nowMs - *lastSampleMs_ < config_.sampleMs) return false;
  lastSampleMs_ = nowMs;

  uint32_t sum = 0;
  for (int index = 0; index < config_.samples; ++index) sum += input_.read();
  const bool firstReading = !raw_.has_value();
  raw_ = static_cast<int>(sum / static_cast<uint32_t>(config_.samples));

  const bool previousAlarm = alarm_;
  alarm_ = core::gasAlarm(alarm_, *raw_, config_.onRaw, config_.offRaw);
  if (alarm_ != previousAlarm && alarmChangeHandler_) alarmChangeHandler_(alarm_);

  if (firstReading || alarm_ != previousAlarm || nowMs - lastPublishMs_ >= config_.publishMs) {
    lastPublishMs_ = nowMs;
    return true;
  }
  return false;
}

bool GasSensorDevice::onCommand(const std::string&, std::string& error) {
  error = "cảm biến chỉ đọc, không nhận lệnh";
  return false;
}

std::string GasSensorDevice::stateJson() const {
  if (!raw_) return protocol::encodeGasWarmup();
  return protocol::encodeGasState(*raw_, alarm_);
}

}  // namespace domisen::services
