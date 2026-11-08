#include <shared/services/TouchToggle.h>

namespace domisen::services {

TouchToggle::TouchToggle(BinarySensorDevice& button, DimmableLight& light) : button_(button), light_(light) {}

void TouchToggle::begin() {
  button_.setRunsOffline(true);
  button_.onChange([this](bool pressed) {
    if (pressed) light_.setOn(!light_.isOn());
  });
}

}  // namespace domisen::services
