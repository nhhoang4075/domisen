#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// Một chân GPIO điều khiển relay. activeLow: module relay hút khi chân IN xuống mức thấp (rất hay gặp).
class Esp32DigitalOut : public core::DigitalOutput {
 public:
  Esp32DigitalOut(int pin, bool activeLow);
  void begin();
  void write(bool on) override;

 private:
  int pin_;
  bool activeLow_;
};

}  // namespace domisen::adapters::esp32
