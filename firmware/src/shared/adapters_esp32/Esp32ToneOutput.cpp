#include <Arduino.h>
#include <shared/adapters_esp32/Esp32ToneOutput.h>

namespace domisen::adapters::esp32 {

static const int kResolutionBits = 8;
static const uint32_t kHalfDuty = 1u << (kResolutionBits - 1);  // 50 %

Esp32ToneOutput::Esp32ToneOutput(int pin, int channel, int toneHz, bool idleHigh)
    : pin_(pin), channel_(channel), toneHz_(toneHz), idleHigh_(idleHigh) {}

void Esp32ToneOutput::begin() {
  ledcSetup(channel_, toneHz_, kResolutionBits);
  silence();
}

void Esp32ToneOutput::write(bool on) {
  if (on == sounding_) return;
  if (on) {
    ledcAttachPin(pin_, channel_);
    ledcWrite(channel_, kHalfDuty);
    sounding_ = true;
  } else {
    silence();
  }
}

void Esp32ToneOutput::silence() {
  ledcDetachPin(pin_);
  digitalWrite(pin_, idleHigh_ ? HIGH : LOW);  // đặt mức trước khi thành đầu ra, để không có xung lạ lúc bật nguồn
  pinMode(pin_, OUTPUT);
  sounding_ = false;
}

}  // namespace domisen::adapters::esp32
