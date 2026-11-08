#pragma once
#include <shared/core/Ports.h>

namespace domisen::adapters::fake {

class FakeTextDisplay : public core::TextDisplay {
 public:
  void show(const std::vector<std::string>& lines) override {
    last = lines;
    history.push_back(lines);
  }

  std::vector<std::string> last;
  std::vector<std::vector<std::string>> history;
};

}  // namespace domisen::adapters::fake
