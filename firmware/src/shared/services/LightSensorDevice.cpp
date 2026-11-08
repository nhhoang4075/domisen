#include <algorithm>
#include <shared/core/LightSensor.h>
#include <shared/protocol/LightSensor.h>
#include <shared/services/LightSensorDevice.h>

namespace domisen::services {

LightSensorDevice::LightSensorDevice(std::string id, core::AnalogInput& input, Config config)
    : id_(std::move(id)), input_(input), config_(config) {
  // Ngưỡng tắt thấp hơn ngưỡng bật thì mất độ trễ và dark có thể kẹt bật: ép về bằng ngưỡng bật.
  config_.brightAtOrAboveRaw = std::max(config_.brightAtOrAboveRaw, config_.darkBelowRaw);
  config_.samples = std::max(config_.samples, 1);
}

bool LightSensorDevice::tick(uint32_t nowMs) {
  if (lastSampleMs_ && nowMs - *lastSampleMs_ < config_.sampleMs) return false;
  lastSampleMs_ = nowMs;

  uint32_t sum = 0;
  for (int index = 0; index < config_.samples; ++index) sum += input_.read();
  const bool firstReading = !lightRaw_.has_value();
  lightRaw_ = core::normalizeLight(static_cast<int>(sum / static_cast<uint32_t>(config_.samples)),
                                   config_.aoFallsWhenBright);

  const bool previousDark = dark_;
  dark_ = core::lightIsDark(dark_, *lightRaw_, config_.darkBelowRaw, config_.brightAtOrAboveRaw);

  if (firstReading || dark_ != previousDark || nowMs - lastPublishMs_ >= config_.publishMs) {
    lastPublishMs_ = nowMs;
    return true;
  }
  return false;
}

bool LightSensorDevice::onCommand(const std::string&, std::string& error) {
  error = "cảm biến chỉ đọc, không nhận lệnh";
  return false;
}

std::string LightSensorDevice::stateJson() const {
  if (!lightRaw_) return protocol::encodeLightSensorUnavailable();
  return protocol::encodeLightSensorState(*lightRaw_, dark_);
}

}  // namespace domisen::services
