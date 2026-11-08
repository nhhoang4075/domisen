#include <shared/services/GasSafety.h>

namespace domisen::services {

GasSafety::GasSafety(GasSensorDevice& gas, SwitchDevice& buzzer, SwitchDevice& exhaustFan)
    : gas_(gas), buzzer_(buzzer), exhaustFan_(exhaustFan) {}

void GasSafety::begin() {
  gas_.onAlarmChange([this](bool alarm) { onAlarmChange(alarm); });
}

void GasSafety::onAlarmChange(bool alarm) {
  if (alarm) {
    buzzer_.setOn(true);
    exhaustFan_.setOn(true);
    exhaustFan_.lockOn("đang có gas, không tắt quạt hút được");
  } else {
    exhaustFan_.unlock();
    exhaustFan_.setOn(false);
    buzzer_.setOn(false);
  }
}

}  // namespace domisen::services
