#pragma once
#include <shared/core/Ports.h>

#include <deque>

namespace domisen::adapters::fake {

class FakeCardReader : public core::CardReader {
 public:
  // Mỗi lần poll lấy một kết quả đã xếp hàng. Hết hàng thì như đầu đọc không có thẻ (rỗng).
  std::optional<std::string> poll() override {
    polls++;
    if (queue.empty()) return std::nullopt;
    std::optional<std::string> next = queue.front();
    queue.pop_front();
    return next;
  }

  void push(const std::string& uid) { queue.push_back(uid); }  // lần poll sau thấy thẻ này
  void pushEmpty() { queue.push_back(std::nullopt); }          // lần poll sau không thấy thẻ

  std::deque<std::optional<std::string>> queue;
  int polls = 0;
};

}  // namespace domisen::adapters::fake
