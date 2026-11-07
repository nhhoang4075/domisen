// Nguyên lý của servo SG90 điều khiển bằng độ rộng xung. Không biết JSON, MQTT hay chân GPIO.
// Xem docs/mqtt-topics.md, mục "Servo: cửa, rèm, giàn phơi".
#pragma once
#include <algorithm>

namespace domisen::core {

constexpr float kServoPeriodMs = 20.0f;   // chu kỳ 20 ms (50 Hz)
constexpr float kServoMinPulseMs = 0.5f;  // xung ở 0 độ
constexpr float kServoSpanMs = 1.9f;      // xung tăng thêm khi quay đến 180 độ (tới 2,4 ms)
constexpr float kServoMaxAngle = 180.0f;

// Góc (độ) ra duty của PwmOutput, duty nằm trong 0..1. Góc ngoài 0..180 bị cắt về khoảng đó.
inline float servoDuty(float angleDeg) {
  const float angle = std::clamp(angleDeg, 0.0f, kServoMaxAngle);
  const float pulseMs = kServoMinPulseMs + kServoSpanMs * angle / kServoMaxAngle;
  return pulseMs / kServoPeriodMs;
}

}  // namespace domisen::core
