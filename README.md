# Domisen

Mô hình nhà thông minh dùng 2 ESP32. Các board gửi dữ liệu cảm biến và nhận lệnh qua MQTT,
server lưu dữ liệu và đẩy lên dashboard trên web.

```
ESP32 A (cửa, bếp)  ─┐
                      ├─ WiFi ─ Mosquitto (MQTT) ─ server (Bun + PostgreSQL) ─ web
ESP32 B (khách, ngủ) ─┘
```

| Thư mục | Nội dung |
|---|---|
| `firmware/` | PlatformIO, 2 board `esp32-a` và `esp32-b`, code dùng chung ở `lib/core` |
| `server/` | Nhận MQTT, lưu dữ liệu, API (chưa làm) |
| `web/` | Dashboard (chưa làm) |
| `docs/` | Bảng GPIO, quy ước MQTT, kế hoạch |

## Cài đặt

### macOS

```bash
brew install mosquitto platformio
brew install --cask mqtt-explorer
```

### Windows

1. Cài [VS Code](https://code.visualstudio.com/), rồi cài tiện ích **PlatformIO IDE** (nó tự tải Python và toolchain).
2. Cài driver USB nếu cắm board mà không thấy cổng COM trong Device Manager: CH340 hoặc CP2102, tùy chip in trên board.
3. Cài [MQTT Explorer](https://mqtt-explorer.com/) để xem tin nhắn.

### Broker

Cả nhóm dùng **chung một broker**, chạy trên máy của một người. Mọi người trỏ firmware tới IP máy đó.
Cấu hình Mosquitto để board ngoài máy kết nối được là mở cổng cho cả mạng, nên chỉ làm trên WiFi tin cậy
(WiFi nhà, hotspot điện thoại), không làm ở nơi công cộng. WiFi trường thường chặn kết nối giữa các máy.

## Chạy firmware

```bash
cd firmware
cp include/domisen_config.example.h include/domisen_config.h   # Windows: copy include\domisen_config.example.h include\domisen_config.h
```

Mở `include/domisen_config.h`, điền WiFi và IP máy chạy broker. Rồi nạp:

```bash
pio run -e esp32-a -t upload
pio device monitor
```

Board B thì đổi thành `-e esp32-b`. Trong VS Code dùng thanh dưới của PlatformIO, chọn đúng env.
Bản hiện tại chỉ nối WiFi và broker, nháy LED và gửi uptime, để kiểm tra cả chuỗi chạy được
(xem `docs/mqtt-topics.md`).

## Quy tắc làm việc

- Chân GPIO ghi ở `docs/gpio.md`, topic ghi ở `docs/mqtt-topics.md`. Đổi thì sửa tài liệu trước.
- `firmware/include/domisen_config.h` chứa WiFi và mật khẩu, **không commit**.
- Thư viện đã ghim phiên bản trong `platformio.ini`, đừng tự nâng.
