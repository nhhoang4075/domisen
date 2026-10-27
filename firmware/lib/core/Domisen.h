// Code dùng chung cho cả 2 board: WiFi, MQTT, tự nối lại, trạng thái online/offline.
#pragma once
#include <Arduino.h>

namespace domisen {

using CommandHandler = void (*)(const String& topic, const String& payload);

// nodeId: tên board, dùng làm MQTT client id và trong topic trạng thái.
void begin(const char* nodeId, CommandHandler onMessage);
// Gọi trong loop(). Tự nối lại WiFi và broker khi bị rớt.
void loop();
bool publish(const String& topic, const String& payload, bool retain = false);
// Đăng ký nhận topic, tự đăng ký lại sau mỗi lần nối lại.
void subscribe(const String& topic);

}  // namespace domisen
