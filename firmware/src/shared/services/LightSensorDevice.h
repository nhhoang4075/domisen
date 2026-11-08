// Cảm biến ánh sáng (module quang trở LDR): lấy mẫu trung bình, chuẩn hóa chiều (số lớn là sáng), cờ dark có trễ. Không nhận lệnh.
// Hub gửi state khi tick() trả true: lúc dark đổi và theo chu kỳ.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <optional>

namespace domisen::services {

class LightSensorDevice : public Device {
 public:
  struct Config {
    int darkBelowRaw;            // dark bật khi light_raw < darkBelowRaw
    int brightAtOrAboveRaw;      // dark tắt khi light_raw >= brightAtOrAboveRaw (phải >= darkBelowRaw)
    bool aoFallsWhenBright = false;  // true nếu AO giảm khi sáng hơn: light_raw = 4095 - đọc thô
    uint32_t sampleMs = 1000;    // chu kỳ lấy mẫu
    uint32_t publishMs = 10000;  // chu kỳ gửi state khi dark không đổi
    int samples = 8;             // số mẫu đọc liên tiếp lấy trung bình
  };

  LightSensorDevice(std::string id, core::AnalogInput& input, Config config);

  const std::string& id() const override { return id_; }
  bool tick(uint32_t nowMs) override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  core::AnalogInput& input_;
  Config config_;
  std::optional<uint32_t> lastSampleMs_;
  uint32_t lastPublishMs_ = 0;
  std::optional<int> lightRaw_;  // chưa có: chưa đọc lần nào
  bool dark_ = false;
};

}  // namespace domisen::services
