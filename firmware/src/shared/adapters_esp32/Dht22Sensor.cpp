#include <shared/adapters_esp32/Dht22Sensor.h>

#include <cmath>

namespace domisen::adapters::esp32 {

Dht22Sensor::Dht22Sensor(int pin) : dht_(pin, DHT22) {}

void Dht22Sensor::begin() { dht_.begin(); }

std::optional<core::ClimateReading> Dht22Sensor::read() {
  float humidity = dht_.readHumidity();
  float temp = dht_.readTemperature();
  if (std::isnan(humidity) || std::isnan(temp)) return std::nullopt;
  return core::ClimateReading{temp, humidity};
}

}  // namespace domisen::adapters::esp32
