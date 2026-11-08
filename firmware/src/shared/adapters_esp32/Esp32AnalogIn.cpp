#include <Arduino.h>
#include <shared/adapters_esp32/Esp32AnalogIn.h>

namespace domisen::adapters::esp32 {

Esp32AnalogIn::Esp32AnalogIn(int pin) : pin_(pin) {}

void Esp32AnalogIn::begin() {
  pinMode(pin_, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(pin_, ADC_11db);
}

uint16_t Esp32AnalogIn::read() { return static_cast<uint16_t>(analogRead(pin_)); }

}  // namespace domisen::adapters::esp32
