# Kiến trúc firmware: lõi và adapter

Học theo project Xóm (`packages/core`, `packages/services`, `packages/adapters`). Luật nằm trong file này là nguồn sự thật:
code khác luật là code sai.

## Các phần (`firmware/src/`)

| Thư mục | Chứa | Được include |
|---|---|---|
| `core` | Luật thuần (độ sáng, gamma, chuyển trạng thái đèn) và **port**: `Transport`, `PwmOutput`, `Clock` | Chỉ thư viện chuẩn C++ |
| `protocol` | Tên topic, mã hóa và giải mã JSON của lệnh, trạng thái, lỗi | ArduinoJson, thư viện chuẩn |
| `services` | `Hub` (đăng ký thiết bị, chuyển lệnh, báo trạng thái, nhịp uptime), `Device`, `DimmableLight`, nhận port qua constructor | `core`, `protocol` |
| `adapters_esp32` | Cài đặt port bằng phần cứng thật: `MqttTransport` (WiFi + PubSubClient), `Esp32Pwm`, `ArduinoClock` | `core`, Arduino, PubSubClient |
| `adapters_fake` | Bản giả để test trên laptop: `FakeTransport`, `FakePwm`, `FakeClock` | `core` |
| `guard/`, `comfort/` | Chỉ **lắp ráp**: tạo adapter, tạo thiết bị, đưa vào `Hub` | Tất cả |

## Luật

1. **Lõi thuần.** `core` và `services` không include `Arduino.h`, `WiFi.h`, `PubSubClient.h` hay adapter. Mỗi môi trường trong `platformio.ini` chọn phần được biên dịch bằng `build_src_filter`: board thật lấy lõi + `adapters_esp32`, môi trường `native` lấy lõi + `adapters_fake`. `native` không có `Arduino.h`, nên lõi lỡ dính phần cứng là biên dịch lỗi. `tools/check-purity.sh` kiểm tra thêm.
2. **Cách include:** `#include <core/Ports.h>`, tức `<thư-mục/tên-file>` tính từ `src/`, không dùng đường dẫn `../`.
3. **Một tên, một nguồn.** Topic chỉ được ghép trong `protocol/Topics.h`. Không viết chuỗi `"domisen/..."` ở chỗ khác.
4. **Thiết bị không biết phần cứng.** `DimmableLight` nhận `PwmOutput&`, không biết chân GPIO hay kênh LEDC. Đèn trần và đèn ngủ dùng chung một lớp, chỉ khác id và PWM.
5. **Thiếu cấu hình thì dừng.** Thiếu WiFi hoặc broker thì board báo lỗi và dừng ngay, không âm thầm chạy tiếp.
6. **Mỗi port có contract test** (`test/test_contracts`), chạy cho mọi adapter có thể chạy trên laptop (hiện là bản giả).
   Adapter thật cần board nên chỉ kiểm tra được khi nạp lên thiết bị.
7. **Đổi thiết kế thì sửa file này trước.**

## Port và adapter

| Port | Adapter thật | Bản giả |
|---|---|---|
| `Transport` (MQTT) | `MqttTransport` | `FakeTransport` (ghi lại tin đã gửi, giả lập nhận tin) |
| `PwmOutput` | `Esp32Pwm` (LEDC) | `FakePwm` |
| `Clock` | `ArduinoClock` (`millis()`) | `FakeClock` (tua được) |

## Thêm một thiết bị

1. Nếu là loại mới (relay, servo...): thêm luật thuần ở `core`, mã hóa ở `protocol`, lớp thiết bị ở `services`, có test với bản giả.
   Nếu là loại đã có (thêm một đèn nữa): bỏ qua bước này.
2. Nếu cần loại phần cứng mới: thêm port vào `core/Ports.h`, adapter thật và bản giả, contract test.
3. Trong `src/<board>/main.cpp` (`guard` hoặc `comfort`): tạo adapter, tạo thiết bị với id theo `docs/mqtt-topics.md`, gọi `hub.add(...)`. Hub tự đăng ký topic lệnh, chuyển lệnh, gửi trạng thái.
4. Cập nhật `docs/gpio.md` và `docs/mqtt-topics.md`.

## Chạy kiểm tra

```bash
cd firmware
pio test -e native        # test lõi, protocol, services, contract trên laptop, không cần board
pio run                   # biên dịch cả hai board
tools/check-purity.sh     # lõi không dính phần cứng
```
