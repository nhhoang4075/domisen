#include <Arduino.h>
#include <shared/adapters_esp32/Esp32DigitalOut.h>

namespace domisen::adapters::esp32 {

Esp32DigitalOut::Esp32DigitalOut(int pin, bool activeLow) : pin_(pin), activeLow_(activeLow) {}

void Esp32DigitalOut::begin() {
  write(false);  // đặt mức "tắt" trước, để lúc bật chế độ đầu ra relay không nháy
  pinMode(pin_, OUTPUT);
}

void Esp32DigitalOut::write(bool on) { digitalWrite(pin_, on != activeLow_ ? HIGH : LOW); }

}  // namespace domisen::adapters::esp32
