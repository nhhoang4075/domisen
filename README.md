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
| `firmware/` | PlatformIO, 2 board `guard` và `comfort`. Kiến trúc lõi và adapter, xem [`docs/architecture.md`](docs/architecture.md) |
| `broker/` | Cấu hình Mosquitto (có mật khẩu), chạy giống nhau trên Mac và Windows |
| `server/` | Nhận MQTT, lưu dữ liệu, API (chưa làm) |
| `web/` | Dashboard (chưa làm) |
| `docs/` | Bảng GPIO, quy ước MQTT, kiến trúc, kế hoạch. Mở `docs/giai-thich-code.html` bằng trình duyệt để xem giải thích code bằng hình |

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

### MQTTX là gì

MQTTX là ứng dụng có giao diện để **xem và gửi tin MQTT** bằng tay, dùng thay cho server và web khi chúng chưa có.
Nó không phải broker: Mosquitto chuyển tin, MQTTX chỉ là một "người dùng" nối vào broker như board.
Tạo kết nối với Host `localhost` (máy khác trong nhóm dùng IP máy chạy broker), Port `1883`, Username `domisen`,
Password do người chạy broker đặt, SSL tắt. **Đừng dùng broker `broker.emqx.io` mà MQTTX điền sẵn**, đó là broker công cộng.
Subscribe `domisen/#` để nghe, gửi lệnh bằng ô ở dưới cùng, **để Retain tắt** khi gửi lệnh.
Hướng dẫn đầy đủ kèm các thử nghiệm: tab MQTTX của [`docs/giai-thich-code.html`](docs/giai-thich-code.html).

### Xem tin nhắn không cần giao diện

Có sẵn cùng Mosquitto, thấy mọi tin của dự án:

```bash
mosquitto_sub -h localhost -u domisen -P 'MẬT_KHẨU' -t 'domisen/#' -v
```

### Broker

Cả nhóm dùng **chung một broker**, chạy trên máy của một người. Mọi người trỏ firmware tới IP máy đó.
Cấu hình và cách chạy nằm ở [`broker/`](broker/README.md). Broker mở cổng cho cả mạng và đòi mật khẩu,
nên chỉ bật ở WiFi tin cậy (WiFi nhà, hotspot điện thoại). WiFi trường thường chặn kết nối giữa các máy.

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
Board `comfort` đã điều khiển được đèn trần và đèn ngủ qua MQTT (xem `docs/mqtt-topics.md`).
Test không cần board: `pio test -e native` (trong thư mục `firmware`).

## Soạn code bằng Zed (macOS)

Zed báo `'Arduino.h' file not found` vì clangd chưa biết đường dẫn thư viện ESP32. Build vẫn bình thường. Chạy một lần:

```bash
firmware/tools/setup-editor.sh
```

Rồi mở lại file (hoặc khởi động lại language server).

### VS Code

Mở thư mục `domisen` (không phải thư mục cha). `.vscode/settings.json` đã trỏ IntelliSense vào `firmware/compile_commands.json`, nên chỉ cần chạy `setup-editor.sh` một lần để file đó được sinh ra, rồi mở lại cửa sổ. Dùng phần mở rộng PlatformIO IDE thì không cần bước này.

## Quy tắc làm việc

Luật đầy đủ (kiến trúc, nhánh, PR, cách review) nằm ở [`AGENTS.md`](AGENTS.md). Tóm tắt:

- Chân GPIO ghi ở `docs/gpio.md`, topic ghi ở `docs/mqtt-topics.md`. Đổi thì sửa tài liệu trước.
- `firmware/include/domisen_config.h` chứa WiFi và mật khẩu, **không commit**.
- Thư viện đã ghim phiên bản trong `platformio.ini`, đừng tự nâng.
