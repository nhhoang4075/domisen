// Thiết bị chỉ có bật và tắt (quạt...). Chỉ khác nhau id và DigitalOutput.
// Ngoài lệnh từ server, code trên board (quy tắc an toàn) cũng bật tắt được qua setOn() và có thể khóa không cho tắt.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

namespace domisen::services {

class SwitchDevice : public Device {
 public:
  SwitchDevice(std::string id, core::DigitalOutput& output);

  const std::string& id() const override { return id_; }
  void begin() override;
  // Trả true một lần sau khi setOn() đổi trạng thái, để Hub gửi state mới lên broker.
  bool tick(uint32_t nowMs) override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

  // Bật hoặc tắt từ code trên board, không qua lệnh MQTT.
  void setOn(bool on);
  // Khóa ở trạng thái bật: lệnh tắt bị từ chối với lỗi reason, lệnh bật vẫn nhận. unlock() bỏ khóa.
  void lockOn(std::string reason);
  void unlock();

 private:
  std::string id_;
  core::DigitalOutput& output_;
  bool on_ = false;
  bool changedByBoard_ = false;
  bool locked_ = false;
  std::string lockReason_;
};

}  // namespace domisen::services
