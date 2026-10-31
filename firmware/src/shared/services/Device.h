// Một thiết bị trong nhà (đèn, quạt, servo...). Hub chỉ làm việc với interface này.
#pragma once
#include <string>

namespace domisen::services {

class Device {
 public:
  virtual ~Device() = default;
  // "<phòng>/<thiết-bị>", ví dụ "living/ceiling-light"
  virtual const std::string& id() const = 0;
  // Gọi một lần khi Hub bắt đầu, để đưa đầu ra về trạng thái ban đầu.
  virtual void begin() {}
  // Trả false và điền error nếu không hiểu lệnh. Khi đó trạng thái không đổi.
  virtual bool onCommand(const std::string& payload, std::string& error) = 0;
  virtual std::string stateJson() const = 0;
};

}  // namespace domisen::services
