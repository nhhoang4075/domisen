// Transport thật: WiFi + MQTT (PubSubClient). Tự nối lại WiFi và broker, đăng ký lại topic sau mỗi lần nối lại.
#pragma once
#include <PubSubClient.h>
#include <WiFi.h>
#include <core/Ports.h>

#include <string>
#include <vector>

namespace domisen::adapters::esp32 {

struct MqttConfig {
  std::string clientId;
  std::string hostname;    // tên hiện trên router
  std::string wifiSsid;
  std::string wifiPass;
  std::string host;        // IP hoặc tên miền broker
  uint16_t port = 1883;
  std::string user;        // để trống nếu broker không đòi mật khẩu
  std::string pass;
  std::string willTopic;   // broker tự gửi khi board mất kết nối
  std::string willPayload = "offline";
};

class MqttTransport : public core::Transport {
 public:
  explicit MqttTransport(MqttConfig config);

  // Thiếu WiFi hoặc broker thì báo lỗi và dừng, không chạy tiếp.
  void begin();
  void loop();

  bool publish(const std::string& topic, const std::string& payload, bool retain) override;
  void subscribe(const std::string& topic) override;
  bool connected() const override;
  void onMessage(MessageHandler handler) override { messageHandler_ = std::move(handler); }
  void onConnect(ConnectHandler handler) override { connectHandler_ = std::move(handler); }

 private:
  bool connectBroker();

  MqttConfig config_;
  WiFiClient net_;
  mutable PubSubClient mqtt_;
  MessageHandler messageHandler_;
  ConnectHandler connectHandler_;
  std::vector<std::string> subscriptions_;
  unsigned long lastTry_ = 0;
  unsigned long backoffMs_ = 1000;  // tăng dần tới 15 giây
};

}  // namespace domisen::adapters::esp32
