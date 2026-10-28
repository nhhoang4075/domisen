// Transport giả: ghi lại tin đã gửi, giả lập nối, ngắt và nhận tin. Hành vi giống adapter thật.
#pragma once
#include <shared/core/Ports.h>

#include <algorithm>
#include <string>
#include <vector>

namespace domisen::adapters::fake {

struct Published {
  std::string topic;
  std::string payload;
  bool retain;
};

class FakeTransport : public core::Transport {
 public:
  bool publish(const std::string& topic, const std::string& payload, bool retain) override {
    if (!connected_) return false;
    published.push_back({topic, payload, retain});
    return true;
  }
  void subscribe(const std::string& topic) override {
    if (std::find(subscriptions.begin(), subscriptions.end(), topic) == subscriptions.end()) {
      subscriptions.push_back(topic);
    }
  }
  bool connected() const override { return connected_; }
  void onMessage(MessageHandler handler) override { messageHandler_ = std::move(handler); }
  void onConnect(ConnectHandler handler) override { connectHandler_ = std::move(handler); }

  // ---- điều khiển trong test ----
  void simulateConnect() {
    connected_ = true;
    if (connectHandler_) connectHandler_();
  }
  void simulateDisconnect() { connected_ = false; }
  // Chỉ chuyển tin tới handler nếu đã đăng ký topic đó, như broker thật.
  void simulateMessage(const std::string& topic, const std::string& payload) {
    if (!connected_ || !messageHandler_) return;
    if (std::find(subscriptions.begin(), subscriptions.end(), topic) == subscriptions.end()) return;
    messageHandler_(topic, payload);
  }

  // ---- đọc kết quả trong test ----
  int count(const std::string& topic) const {
    return static_cast<int>(std::count_if(published.begin(), published.end(),
                                          [&](const Published& p) { return p.topic == topic; }));
  }
  const Published* last(const std::string& topic) const {
    for (auto it = published.rbegin(); it != published.rend(); ++it) {
      if (it->topic == topic) return &*it;
    }
    return nullptr;
  }

  std::vector<Published> published;
  std::vector<std::string> subscriptions;

 private:
  bool connected_ = false;
  MessageHandler messageHandler_;
  ConnectHandler connectHandler_;
};

}  // namespace domisen::adapters::fake
