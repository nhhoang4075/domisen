// Đèn trần phòng khách (#1): LED trắng, GPIO25 qua điện trở 220 ohm, chỉnh độ sáng bằng PWM.
// Topic: domisen/living/ceiling-light/cmd (nhận) và .../state (gửi, retain). Xem docs/mqtt-topics.md.
#pragma once
#include <Arduino.h>

namespace ceiling_light {
void begin();
void onCommand(const String& payload);
void publishState();
extern const char* CMD_TOPIC;
}  // namespace ceiling_light
