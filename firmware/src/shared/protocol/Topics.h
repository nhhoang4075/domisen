// Nguồn duy nhất cho tên topic. Quy ước ở docs/mqtt-topics.md.
#pragma once
#include <string>

namespace domisen::protocol {

inline const std::string kRoot = "domisen";

// deviceId dạng "<phòng>/<thiết-bị>", ví dụ "living/ceiling-light"
inline std::string cmdTopic(const std::string& deviceId) { return kRoot + "/" + deviceId + "/cmd"; }
inline std::string stateTopic(const std::string& deviceId) { return kRoot + "/" + deviceId + "/state"; }
inline std::string errorTopic(const std::string& deviceId) { return kRoot + "/" + deviceId + "/error"; }

// Board (node): "guard", "comfort"
inline std::string nodeStatusTopic(const std::string& node) { return kRoot + "/node/" + node + "/status"; }
inline std::string nodeUptimeTopic(const std::string& node) { return kRoot + "/node/" + node + "/uptime"; }
inline std::string nodePingTopic(const std::string& node) { return kRoot + "/node/" + node + "/ping"; }
inline std::string nodePongTopic(const std::string& node) { return kRoot + "/node/" + node + "/pong"; }

}  // namespace domisen::protocol
