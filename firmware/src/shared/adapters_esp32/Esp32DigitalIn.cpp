#include <Arduino.h>
#include <shared/adapters_esp32/Esp32DigitalIn.h>

namespace domisen::adapters::esp32 {

Esp32DigitalIn::Esp32DigitalIn(int pin) : pin_(pin) {}

void Esp32DigitalIn::begin() { pinMode(pin_, INPUT); }

bool Esp32DigitalIn::read() { return digitalRead(pin_) == HIGH; }

}  // namespace domisen::adapters::esp32
