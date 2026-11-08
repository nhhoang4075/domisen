// Cảm biến gas MQ-2: làm nóng, lấy mẫu trung bình, báo động có trễ. Không nhận lệnh.
// Hub gửi state khi tick() trả true: lúc alarm đổi và theo chu kỳ.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <functional>
#include <optional>

namespace domisen::services {

class GasSensorDevice : public Device {
 public:
  struct Config {
    int onRaw;                   // alarm bật khi raw >= onRaw
    int offRaw;                  // alarm tắt khi raw < offRaw (phải thấp hơn onRaw)
    uint32_t warmupMs = 60000;   // làm nóng sau khi khởi động, chưa đọc
    uint32_t sampleMs = 1000;    // chu kỳ lấy mẫu
    uint32_t publishMs = 5000;   // chu kỳ gửi state khi alarm không đổi
    int samples = 8;             // số mẫu đọc liên tiếp lấy trung bình
  };

  GasSensorDevice(std::string id, core::AnalogInput& input, core::Clock& clock, Config config);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool runsOffline() const override { return true; }  // an toàn: vẫn lấy mẫu khi mất kết nối broker
  bool tick(uint32_t nowMs) override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

  bool alarm() const { return alarm_; }
  // Gọi ngay khi alarm đổi (cả lúc mất kết nối broker). Quy tắc an toàn đăng ký ở đây.
  void onAlarmChange(std::function<void(bool alarm)> handler) { alarmChangeHandler_ = std::move(handler); }

 private:
  std::string id_;
  core::AnalogInput& input_;
  core::Clock& clock_;
  Config config_;
  uint32_t bootMs_ = 0;
  std::optional<uint32_t> lastSampleMs_;
  uint32_t lastPublishMs_ = 0;
  std::optional<int> raw_;  // chưa có: đang làm nóng
  bool alarm_ = false;
  std::function<void(bool)> alarmChangeHandler_;
};

}  // namespace domisen::services
