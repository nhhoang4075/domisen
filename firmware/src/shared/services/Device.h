// Một thiết bị trong nhà (đèn, quạt, servo...). Hub chỉ làm việc với interface này.
#pragma once
#include <cstdint>
#include <string>

namespace domisen::services {

class Device {
 public:
  virtual ~Device() = default;
  // "<phòng>/<thiết-bị>", ví dụ "living/ceiling-light"
  virtual const std::string& id() const = 0;
  // Gọi một lần khi Hub bắt đầu, để đưa đầu ra về trạng thái ban đầu.
  virtual void begin() {}
  // Hub gọi mỗi vòng loop() khi đã nối broker. Trả true nếu cần gửi lại state (cảm biến gửi theo chu kỳ).
  virtual bool tick(uint32_t nowMs) { return false; }
  // Thiết bị an toàn cần chạy cả khi mất kết nối broker (ví dụ cảm biến gas): Hub vẫn gọi tick() lúc đó, nhưng không gửi state.
  virtual bool runsOffline() const { return false; }
  // Trả false và điền error nếu không hiểu lệnh. Khi đó trạng thái không đổi.
  virtual bool onCommand(const std::string& payload, std::string& error) = 0;
  virtual std::string stateJson() const = 0;
  // Hub gửi state với cờ retain này. Mặc định true; thiết bị báo sự kiện (quẹt thẻ) trả false để người nối sau không thấy sự kiện cũ.
  virtual bool retainState() const { return true; }
  // Hub có gửi state của thiết bị này lúc nối (lại) broker không. Mặc định true; sự kiện thì không gửi lại.
  virtual bool publishOnConnect() const { return true; }
};

}  // namespace domisen::services
