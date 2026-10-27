# broker

Cấu hình Mosquitto của Domisen, chạy giống nhau trên macOS và Windows. Chỉ **một máy** trong nhóm chạy broker,
các máy khác trỏ `MQTT_HOST` trong firmware tới IP máy đó.

```bash
cd broker
mosquitto_passwd -c passwd domisen    # một lần, hỏi mật khẩu 2 lần
mosquitto -c mosquitto.conf           # giữ cửa sổ này mở; Ctrl+C để tắt
```

- `passwd` không đưa lên git. Gửi mật khẩu cho nhóm qua tin nhắn riêng.
- Chỉ mở broker ở WiFi tin cậy (WiFi nhà, hotspot điện thoại).
- Windows: cài Mosquitto từ https://mosquitto.org/download/, thêm thư mục cài đặt vào PATH, rồi chạy cùng các lệnh trên.
- Nếu đã bật Mosquitto qua `brew services`, tắt trước để khỏi trùng cổng 1883: `brew services stop mosquitto`.
