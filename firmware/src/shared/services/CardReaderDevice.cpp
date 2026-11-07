#include <shared/protocol/CardReader.h>
#include <shared/services/CardReaderDevice.h>

namespace domisen::services {

CardReaderDevice::CardReaderDevice(std::string id, core::CardReader& reader, uint32_t repeatMs)
    : id_(std::move(id)), reader_(reader), repeatMs_(repeatMs) {}

bool CardReaderDevice::tick(uint32_t nowMs) {
  std::optional<std::string> uid = reader_.poll();
  if (!uid) {
    seenEmpty_ = true;  // không có thẻ: thẻ vừa rồi (nếu có) đã rời đi
    return false;
  }
  // Gửi nếu: UID khác lần gửi trước, hoặc thẻ đã rời đi rồi quay lại, hoặc đã qua repeatMs_ kể từ lần gửi trước.
  bool send = !lastUid_ || *uid != *lastUid_ || seenEmpty_ || nowMs - lastSentMs_ >= repeatMs_;
  if (!send) return false;
  lastUid_ = *uid;
  lastSentMs_ = nowMs;
  seenEmpty_ = false;
  return true;
}

bool CardReaderDevice::onCommand(const std::string&, std::string& error) {
  error = "đầu đọc thẻ chỉ đọc, không nhận lệnh";
  return false;
}

std::string CardReaderDevice::stateJson() const {
  if (!lastUid_) return protocol::encodeCardReaderEmpty();
  return protocol::encodeCardReaderState(*lastUid_);
}

}  // namespace domisen::services
