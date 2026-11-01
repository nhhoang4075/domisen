#pragma once
#include <DHT.h>
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// DHT22 (AM2302) dùng thư viện DHT của Adafruit. Đọc chậm nhất 2 giây một lần.
class Dht22Sensor : public core::ClimateSensor {
 public:
  explicit Dht22Sensor(int pin);
  void begin();
  std::optional<core::ClimateReading> read() override;

 private:
  DHT dht_;
};

}  // namespace domisen::adapters::esp32
