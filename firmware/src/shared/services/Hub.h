// Trung tâm của một board: nhận thiết bị, đăng ký topic lệnh, chuyển lệnh, báo trạng thái, nhịp uptime.
// Thêm thiết bị mới chỉ cần hub.add(...), không phải sửa Hub.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <string>
#include <vector>

namespace domisen::services {

class Hub {
 public:
  Hub(std::string node, core::Transport& transport, core::Clock& clock, uint32_t heartbeatMs = 5000);

  void add(Device& device);  // gọi trước begin()
  void begin();
  void loop();               // gọi trong loop() của board

 private:
  void handleConnect();
  void handleMessage(const std::string& topic, const std::string& payload);
  void publishState(Device& device);

  std::string node_;
  core::Transport& transport_;
  core::Clock& clock_;
  uint32_t heartbeatMs_;
  uint32_t lastBeat_ = 0;
  uint32_t lastTick_ = 0;   // lần loop() trước, để cộng dồn uptime
  uint64_t uptimeMs_ = 0;   // không quay vòng như millis() (khoảng 49,7 ngày)
  std::vector<Device*> devices_;
};

}  // namespace domisen::services
