// Sao chép file này thành domisen_config.h rồi điền thông tin của bạn.
// domisen_config.h nằm trong .gitignore, không đưa lên git.
#pragma once

#define WIFI_SSID "ten-wifi"
#define WIFI_PASS "mat-khau-wifi"

// IP máy chạy Mosquitto (macOS: ipconfig getifaddr en0, Windows: ipconfig)
#define MQTT_HOST "192.168.1.239"
#define MQTT_PORT 1883
// Để trống nếu broker chưa bật mật khẩu
#define MQTT_USER ""
#define MQTT_PASS ""
