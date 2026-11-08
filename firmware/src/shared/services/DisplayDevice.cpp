#include <shared/core/DisplayText.h>
#include <shared/protocol/Display.h>
#include <shared/services/DisplayDevice.h>

namespace domisen::services {

DisplayDevice::DisplayDevice(std::string id, core::TextDisplay& display)
    : id_(std::move(id)), display_(display) {}

void DisplayDevice::begin() {
  text_.clear();
  lines_.clear();
  display_.show(lines_);
}

bool DisplayDevice::onCommand(const std::string& payload, std::string& error) {
  std::string text;
  if (!protocol::decodeDisplayCommand(payload, text, error)) return false;

  std::vector<std::string> lines;
  if (!core::splitDisplayText(text, lines, error)) return false;

  text_ = std::move(text);
  lines_ = std::move(lines);
  display_.show(lines_);
  return true;
}

std::string DisplayDevice::stateJson() const { return protocol::encodeDisplayState(text_); }

}  // namespace domisen::services
