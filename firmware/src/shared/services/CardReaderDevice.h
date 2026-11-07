// Đầu đọc thẻ. Quẹt thẻ là sự kiện, không phải trạng thái: gửi UID một lần, không retain,
// không gửi lại khi nối lại broker. Không nhận lệnh.
#pragma once
#include <shared/core/Ports.h>
#include <shared/services/Device.h>

#include <optional>

namespace domisen::services {

class CardReaderDevice : public Device {
 public:
  CardReaderDevice(std::string id, core::CardReader& reader, uint32_t repeatMs = 2000);

  const std::string& id() const override { return id_; }
  // Trả true khi có một lần quẹt cần gửi (quy tắc chống lặp nằm trong .cpp).
  bool tick(uint32_t nowMs) override;
  bool onCommand(const std::string& payload, std::string& error) override;
  std::string stateJson() const override;
  bool retainState() const override { return false; }
  bool publishOnConnect() const override { return false; }

 private:
  std::string id_;
  core::CardReader& reader_;
  uint32_t repeatMs_;
  std::optional<std::string> lastUid_;  // UID đã gửi lần trước
  uint32_t lastSentMs_ = 0;
  bool seenEmpty_ = false;  // đã có lần poll rỗng kể từ lần gửi trước: thẻ đã rời đầu đọc
};

}  // namespace domisen::services
