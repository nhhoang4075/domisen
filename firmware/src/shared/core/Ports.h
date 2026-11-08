// Port: những thứ lõi cần từ bên ngoài. Lõi chỉ biết interface, adapter mới biết phần cứng.
#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace domisen::core {

class Clock {
 public:
  virtual ~Clock() = default;
  virtual uint32_t nowMs() const = 0;
};

// Đầu ra PWM. duty nằm trong 0.0 .. 1.0, adapter tự giới hạn và đổi sang độ phân giải của phần cứng.
class PwmOutput {
 public:
  virtual ~PwmOutput() = default;
  virtual void write(float duty) = 0;
};

struct ClimateReading {
  float temp;      // độ C
  float humidity;  // % độ ẩm tương đối
};

// Cảm biến nhiệt độ và độ ẩm. Trả nullopt nếu lần đọc này lỗi (adapter không được trả NaN).
class ClimateSensor {
 public:
  virtual ~ClimateSensor() = default;
  virtual std::optional<ClimateReading> read() = 0;
};

// Đầu ra bật tắt (relay). on là nghĩa logic: adapter lo chuyện relay chạy mức thấp hay mức cao.
class DigitalOutput {
 public:
  virtual ~DigitalOutput() = default;
  virtual void write(bool on) = 0;
};

// Đầu vào số. Adapter trả true khi chân ở mức HIGH.
class DigitalInput {
 public:
  virtual ~DigitalInput() = default;
  virtual bool read() = 0;
};

// Đầu vào analog (ADC 12 bit). read() trả giá trị thô 0 đến 4095, một mẫu mỗi lần gọi.
class AnalogInput {
 public:
  virtual ~AnalogInput() = default;
  virtual uint16_t read() = 0;
};

// Thanh đèn nhiều bóng nhận cùng một màu (WS2812). Mỗi kênh 0..255 là màu thực tế đã nhân độ sáng:
// lõi tính hết, adapter chỉ đẩy ra phần cứng.
class PixelStrip {
 public:
  virtual ~PixelStrip() = default;
  virtual void setColor(uint8_t r, uint8_t g, uint8_t b) = 0;
};

// Đầu đọc thẻ (RC522). poll() trả UID của thẻ mới đưa vào đầu đọc, dạng hex chữ hoa không dấu cách (ví dụ "A3F2197C"),
// hoặc rỗng nếu không có thẻ mới. Chống lặp khi giữ thẻ trên đầu đọc là việc của thiết bị, không phải của port.
class CardReader {
 public:
  virtual ~CardReader() = default;
  virtual std::optional<std::string> poll() = 0;
};

// Màn hình ký tự. Mỗi phần tử trong danh sách là một dòng.
class TextDisplay {
 public:
  virtual ~TextDisplay() = default;
  virtual void show(const std::vector<std::string>& lines) = 0;
};

// Đường truyền tin nhắn (MQTT). Adapter lo nối lại và đăng ký lại topic sau mỗi lần nối lại.
class Transport {
 public:
  using MessageHandler = std::function<void(const std::string& topic, const std::string& payload)>;
  using ConnectHandler = std::function<void()>;

  virtual ~Transport() = default;
  // Trả false nếu chưa nối được (tin không được gửi).
  virtual bool publish(const std::string& topic, const std::string& payload, bool retain) = 0;
  // Có thể gọi trước khi nối. Đăng ký được giữ lại qua các lần nối lại.
  virtual void subscribe(const std::string& topic) = 0;
  virtual bool connected() const = 0;
  virtual void onMessage(MessageHandler handler) = 0;
  // Gọi mỗi lần (nối lại) vào broker xong.
  virtual void onConnect(ConnectHandler handler) = 0;
};

}  // namespace domisen::core
