#include <shared/adapters_esp32/MqttTransport.h>

#include <algorithm>

namespace domisen::adapters::esp32 {

// Thời gian chặn tối đa mỗi lần thử nối broker (docs/architecture.md, mục "Nối broker không chặn lâu"). Trong lúc chặn,
// hub.loop() không chạy nên mọi thiết bị, kể cả quy tắc an toàn gas, không được tick.
// Mặc định của thư viện là 3 giây (nối TCP) và 15 giây (chờ CONNACK). LAN chỉ cần vài mili giây nên 1 giây và 2 giây là dư.
static const int32_t kBrokerConnectTimeoutMs = 1000;
static const uint16_t kBrokerReplyTimeoutSec = 2;

MqttTransport::MqttTransport(MqttConfig config) : config_(std::move(config)), mqtt_(net_) {}

void MqttTransport::begin() {
  if (config_.wifiSsid.empty() || config_.host.empty()) {
    Serial.println("[domisen] thiếu WIFI_SSID hoặc MQTT_HOST, dừng.");
    while (true) delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(config_.hostname.c_str());
  WiFi.setAutoReconnect(true);
  WiFi.begin(config_.wifiSsid.c_str(), config_.wifiPass.c_str());

  mqtt_.setServer(config_.host.c_str(), config_.port);
  mqtt_.setBufferSize(512);
  mqtt_.setSocketTimeout(kBrokerReplyTimeoutSec);
  mqtt_.setCallback([this](char* topic, uint8_t* data, unsigned int len) {
    if (messageHandler_) messageHandler_(topic, std::string(reinterpret_cast<char*>(data), len));
  });
}

bool MqttTransport::connectBroker() {
  // Nối TCP trước với thời gian chờ ngắn: PubSubClient::connect thấy TCP đã nối thì bỏ qua bước nối TCP (mặc định chờ 3 giây).
  if (!net_.connected() && !net_.connect(config_.host.c_str(), config_.port, kBrokerConnectTimeoutMs)) return false;
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
  const bool ok = connectBroker();
  const unsigned long tookMs = millis() - lastTry_;  // thời gian vòng loop() bị chặn lần này
  if (ok) {
    Serial.printf("[%s] đã nối broker %s (mất %lu ms)\n", config_.clientId.c_str(), config_.host.c_str(), tookMs);
  } else {
    backoffMs_ = std::min<unsigned long>(backoffMs_ * 2, 15000);
    Serial.printf("[%s] chưa nối được broker (mã %d, chặn %lu ms), thử lại sau %lu ms\n", config_.clientId.c_str(),
                  mqtt_.state(), tookMs, backoffMs_);
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
