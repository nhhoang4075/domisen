#include <domisen/adapters/esp32/MqttTransport.h>

#include <algorithm>

namespace domisen::adapters::esp32 {

MqttTransport::MqttTransport(MqttConfig config) : config_(std::move(config)), mqtt_(net_) {}

void MqttTransport::begin() {
  if (config_.wifiSsid.empty() || config_.host.empty()) {
    Serial.println("[domisen] thiếu WIFI_SSID hoặc MQTT_HOST trong include/domisen_config.h, dừng.");
    while (true) delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config_.hostname.c_str());
  WiFi.setAutoReconnect(true);
  WiFi.begin(config_.wifiSsid.c_str(), config_.wifiPass.c_str());

  mqtt_.setServer(config_.host.c_str(), config_.port);
  mqtt_.setBufferSize(512);
  mqtt_.setCallback([this](char* topic, uint8_t* data, unsigned int len) {
    if (messageHandler_) messageHandler_(topic, std::string(reinterpret_cast<char*>(data), len));
  });
}

bool MqttTransport::connectBroker() {
  const char* user = config_.user.empty() ? nullptr : config_.user.c_str();
  const char* pass = config_.user.empty() ? nullptr : config_.pass.c_str();
  if (!mqtt_.connect(config_.clientId.c_str(), user, pass, config_.willTopic.c_str(), 1, true,
                     config_.willPayload.c_str())) {
    return false;
  }
  for (const std::string& topic : subscriptions_) mqtt_.subscribe(topic.c_str());
  if (connectHandler_) connectHandler_();
  return true;
}

void MqttTransport::loop() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt_.connected()) {
    backoffMs_ = 1000;
    mqtt_.loop();
    return;
  }
  if (millis() - lastTry_ < backoffMs_) return;
  lastTry_ = millis();
  if (connectBroker()) {
    Serial.printf("[%s] đã nối broker %s\n", config_.clientId.c_str(), config_.host.c_str());
  } else {
    backoffMs_ = std::min<unsigned long>(backoffMs_ * 2, 15000);
    Serial.printf("[%s] chưa nối được broker (mã %d), thử lại sau %lu ms\n", config_.clientId.c_str(),
                  mqtt_.state(), backoffMs_);
  }
}

bool MqttTransport::publish(const std::string& topic, const std::string& payload, bool retain) {
  return mqtt_.connected() && mqtt_.publish(topic.c_str(), payload.c_str(), retain);
}

void MqttTransport::subscribe(const std::string& topic) {
  if (std::find(subscriptions_.begin(), subscriptions_.end(), topic) == subscriptions_.end()) {
    subscriptions_.push_back(topic);
  }
  if (mqtt_.connected()) mqtt_.subscribe(topic.c_str());
}

bool MqttTransport::connected() const { return mqtt_.connected(); }

}  // namespace domisen::adapters::esp32
