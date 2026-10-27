#include <Arduino.h>
#include <domisen/adapters/esp32/Esp32Pwm.h>

#include <algorithm>

namespace domisen::adapters::esp32 {

Esp32Pwm::Esp32Pwm(int pin, int channel, int freqHz, int resolutionBits)
    : pin_(pin), channel_(channel), freqHz_(freqHz), bits_(resolutionBits) {}

void Esp32Pwm::begin() {
  ledcSetup(channel_, freqHz_, bits_);
  ledcAttachPin(pin_, channel_);
  ledcWrite(channel_, 0);
}

void Esp32Pwm::write(float duty) {
  const uint32_t maxDuty = (1u << bits_) - 1;
  ledcWrite(channel_, static_cast<uint32_t>(std::clamp(duty, 0.0f, 1.0f) * maxDuty + 0.5f));
}

}  // namespace domisen::adapters::esp32
