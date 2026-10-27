#include <protocol/Error.h>
#include <protocol/Topics.h>
#include <services/Hub.h>

namespace domisen::services {

Hub::Hub(std::string node, core::Transport& transport, core::Clock& clock, uint32_t heartbeatMs)
    : node_(std::move(node)), transport_(transport), clock_(clock), heartbeatMs_(heartbeatMs) {}

void Hub::add(Device& device) { devices_.push_back(&device); }

void Hub::begin() {
  transport_.onMessage([this](const std::string& topic, const std::string& payload) { handleMessage(topic, payload); });
  transport_.onConnect([this] { handleConnect(); });
  transport_.subscribe(protocol::nodePingTopic(node_));
  for (Device* device : devices_) {
    device->begin();
    transport_.subscribe(protocol::cmdTopic(device->id()));
  }
}

void Hub::handleConnect() {
  transport_.publish(protocol::nodeStatusTopic(node_), "online", true);
  for (Device* device : devices_) publishState(*device);
  lastBeat_ = clock_.nowMs();
}

void Hub::publishState(Device& device) {
  transport_.publish(protocol::stateTopic(device.id()), device.stateJson(), true);
}

void Hub::handleMessage(const std::string& topic, const std::string& payload) {
  if (topic == protocol::nodePingTopic(node_)) {
    transport_.publish(protocol::nodePongTopic(node_), payload, false);
    return;
  }
  for (Device* device : devices_) {
    if (topic != protocol::cmdTopic(device->id())) continue;
    std::string error;
    if (device->onCommand(payload, error)) {
      publishState(*device);
    } else {
      transport_.publish(protocol::errorTopic(device->id()), protocol::encodeError(error), false);
    }
    return;
  }
}

void Hub::loop() {
  if (!transport_.connected()) return;
  uint32_t now = clock_.nowMs();
  if (now - lastBeat_ < heartbeatMs_) return;
  lastBeat_ = now;
  transport_.publish(protocol::nodeUptimeTopic(node_), std::to_string(now / 1000), false);
}

}  // namespace domisen::services
