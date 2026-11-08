// Màn hình nhận lệnh văn bản; lệnh sai không đổi state hay nội dung hiển thị.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <vector>

namespace domisen::services {

class DisplayDevice : public Device {
 public:
  DisplayDevice(std::string id, core::TextDisplay& display);

  const std::string& id() const override { return id_; }
  void begin() override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;

 private:
  std::string id_;
  core::TextDisplay& display_;
  std::string text_;
  std::vector<std::string> lines_;
};

}  // namespace domisen::services
