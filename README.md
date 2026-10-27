# Domisen

Mô hình nhà thông minh dùng 2 ESP32. Các board gửi dữ liệu cảm biến và nhận lệnh qua MQTT,
server lưu dữ liệu và đẩy lên dashboard trên web.

```
guard   (ESP32 A: cửa, bếp)   ─┐
                      ├─ WiFi ─ Mosquitto (MQTT) ─ server (Bun + PostgreSQL) ─ web
comfort (ESP32 B: khách, ngủ) ─┘
```

| Thư mục | Nội dung |
|---|---|
| `firmware/` | PlatformIO, 2 board `guard` và `comfort`, code dùng chung ở `lib/core` |
| `server/` | Nhận MQTT, lưu dữ liệu, API (chưa làm) |
| `web/` | Dashboard (chưa làm) |
| `docs/` | Bảng GPIO, quy ước MQTT, kế hoạch |

## Tên board

Hai ESP32 giống hệt nhau về phần cứng, nên đặt tên theo vai trò. Tài liệu cũ (kế hoạch, hướng dẫn dựng mô hình) vẫn gọi A và B.

| Tên | Trong tài liệu cũ | Vai trò |
|---|---|---|
| `guard` | ESP32 A | Cửa, bếp, an ninh: servo cửa, RC522, MC-38, MQ-2, buzzer, OLED, quạt hút |
| `comfort` | ESP32 B | Tiện nghi: đèn, quạt, DHT22, quang trở, PIR, rèm, WS2812, mưa, giàn phơi, nút cảm ứng |

Tên board dùng làm env PlatformIO, thư mục `firmware/src/<tên>/`, MQTT client id, hostname WiFi (`domisen-guard`) và topic `domisen/node/<tên>/status`.
Nên dán nhãn tên lên từng board.

## Cài đặt

### macOS

```bash
brew install mosquitto platformio
brew install --cask mqttx
```

### Windows

1. Cài [VS Code](https://code.visualstudio.com/), rồi cài tiện ích **PlatformIO IDE** (nó tự tải Python và toolchain).
2. Cài driver USB nếu cắm board mà không thấy cổng COM trong Device Manager: CH340 hoặc CP2102, tùy chip in trên board.
3. Cài [MQTTX](https://mqttx.app/) để xem tin nhắn (bản Mac cũng dùng MQTTX).

### Xem tin nhắn không cần giao diện

Có sẵn cùng Mosquitto, thấy mọi tin của dự án:

```bash
mosquitto_sub -h localhost -t 'domisen/#' -v
```

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
pio run -e guard -t upload
pio device monitor
```

Board kia thì đổi thành `-e comfort`. Trong VS Code dùng thanh dưới của PlatformIO, chọn đúng env.
Bản hiện tại chỉ nối WiFi và broker, nháy LED và gửi uptime, để kiểm tra cả chuỗi chạy được
(xem `docs/mqtt-topics.md`).

## Soạn code bằng Zed (macOS)

Zed báo `'Arduino.h' file not found` vì clangd chưa biết đường dẫn thư viện ESP32. Build vẫn bình thường. Chạy một lần:

```bash
firmware/tools/setup-editor.sh
```

Rồi mở lại file (hoặc khởi động lại language server). Dùng VS Code với PlatformIO IDE thì không cần bước này.

## Quy tắc làm việc

- Chân GPIO ghi ở `docs/gpio.md`, topic ghi ở `docs/mqtt-topics.md`. Đổi thì sửa tài liệu trước.
- `firmware/include/domisen_config.h` chứa WiFi và mật khẩu, **không commit**.
- Thư viện đã ghim phiên bản trong `platformio.ini`, đừng tự nâng.
