# Kiến trúc firmware: lõi và adapter

Học theo project Xóm (`packages/core`, `packages/services`, `packages/adapters`). Luật nằm trong file này là nguồn sự thật:
code khác luật là code sai.

## Các phần (`firmware/src/`)

Code dùng chung nằm trong `src/shared/`, điểm vào của từng board nằm cạnh nó (`src/guard/`, `src/comfort/`).

| Thư mục | Chứa | Được include |
|---|---|---|
| `shared/core` | Luật thuần (độ sáng, gamma, chuyển trạng thái đèn) và **port**: `Transport`, `PwmOutput`, `Clock`, `ClimateSensor`, `DigitalOutput`, `DigitalInput`, `AnalogInput`, `PixelStrip`, `TextDisplay`, `CardReader` | Chỉ thư viện chuẩn C++ |
| `shared/protocol` | Tên topic, mã hóa và giải mã JSON của lệnh, trạng thái, lỗi | ArduinoJson, thư viện chuẩn |
| `shared/services` | `Hub` (đăng ký thiết bị, chuyển lệnh, báo trạng thái, nhịp uptime), `Device`, `DimmableLight`, `SpeedFan`, `ClimateDevice`, `SwitchDevice`, `BinarySensorDevice`, `GasSensorDevice`, `GasSafety`, `LightSensorDevice`, `PositionDevice`, `ColorLight`, `DisplayDevice`, `CardReaderDevice`, nhận port qua constructor | `shared/core`, `shared/protocol` |
| `shared/adapters_esp32` | Cài đặt port bằng phần cứng thật: `MqttTransport` (WiFi + PubSubClient), `Esp32Pwm`, `ArduinoClock`, `Dht22Sensor` (thư viện DHT của Adafruit), `Esp32DigitalOut`, `Esp32DigitalIn`, `Esp32AnalogIn`, `Esp32ToneOutput`, `Ws2812Strip`, `Ssd1306Display`, `Rc522Reader` | `shared/core`, Arduino, PubSubClient |
| `shared/adapters_fake` | Bản giả để test trên laptop: `FakeTransport`, `FakePwm`, `FakeClock`, `FakeClimateSensor`, `FakeDigitalOutput`, `FakeDigitalInput`, `FakeAnalogInput`, `FakePixelStrip`, `FakeTextDisplay`, `FakeCardReader` | `shared/core` |
| `guard/`, `comfort/` | Chỉ **lắp ráp**: tạo adapter, tạo thiết bị, đưa vào `Hub` | Tất cả |

## Luật

1. **Lõi thuần.** `core` và `services` không include `Arduino.h`, `WiFi.h`, `PubSubClient.h` hay adapter. Mỗi môi trường trong `platformio.ini` chọn phần được biên dịch bằng `build_src_filter`: board thật lấy lõi + `shared/adapters_esp32`, môi trường `native` lấy lõi + `shared/adapters_fake`. `native` không có `Arduino.h`, nên lõi lỡ dính phần cứng là biên dịch lỗi. `tools/check-purity.sh` kiểm tra thêm.
2. **Cách include:** `#include <shared/core/Ports.h>`, tức đường dẫn tính từ `src/`, không dùng đường dẫn `../`.
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
| `ClimateSensor` (nhiệt độ, độ ẩm) | `Dht22Sensor` | `FakeClimateSensor` (đặt trước giá trị đọc được) |
| `DigitalOutput` (bật tắt: relay) | `Esp32DigitalOut` (lo luôn chuyện relay chạy mức thấp) | `FakeDigitalOutput` |

## Thiết bị chỉ gửi số đo (cảm biến)

Cảm biến không nhận lệnh mà tự gửi trạng thái theo chu kỳ. `Device` có hàm `tick(nowMs)`: Hub gọi mỗi vòng `loop()` khi đã nối broker,
thiết bị tự đo giờ và trả `true` khi cần gửi lại state (Hub gửi retain). Đèn không cần làm gì vì mặc định `tick` trả `false`.
Khi cảm biến đọc lỗi, state gửi giá trị `null` thay vì giữ số cũ, để bên nhận biết cảm biến đang hỏng.
Cảm biến nhận lệnh thì trả lỗi "chỉ đọc" như mọi lệnh sai.

## Thiết bị bật tắt (relay)

`SwitchDevice` dùng cho mọi thiết bị chỉ có bật và tắt (quạt hút bếp...), chỉ khác id và `DigitalOutput`.
Lõi chỉ biết "bật" hay "tắt" theo nghĩa logic: relay chạy mức thấp (active low) hay mức cao là việc của adapter `Esp32DigitalOut`.
Khởi động luôn ở trạng thái tắt, và adapter đặt mức "tắt" trước khi bật chế độ đầu ra để relay không nháy lúc khởi động.
Việc tự bật quạt theo nhiệt độ **không nằm trên board**: đó là luật tiện nghi, chạy trên server (xem `docs/plan`, nguyên tắc "việc an toàn xử lý ngay tại nhà").
Board chỉ nhận lệnh `on` và báo state. Quạt hút khi có gas (an toàn) do board guard tự quyết, mô tả ở mục "Quy tắc an toàn gas" bên dưới.

## Quạt phòng khách chỉnh tốc độ

Quạt phòng khách dùng lớp dịch vụ `SpeedFan`: cùng lệnh `on`/`level` và trạng thái `LightState` với đèn, nhưng luật khác. `core/Fan.h` có `fanDuty` (tuyến tính từ duty tối thiểu, không bù gamma) và `fanNeedsKick` (kick-start (chạy hết ga lúc khởi động): từ đứng yên lên duty thấp thì chạy hết ga một lúc). `SpeedFan` nhận `Clock` để đo thời gian kick và hết kick ở `tick` của Hub. Adapter vẫn là `Esp32Pwm`, bản giả `FakePwm`.

## Port, lớp dịch vụ và adapter của từng thiết bị

Nội dung chi tiết (topic, trường, chân, thư viện) nằm ở `docs/mqtt-topics.md`. Ở đây chỉ ghi phần kiến trúc: port, lớp, chỗ đặt.

| Thiết bị | Port | Lớp dịch vụ | Adapter thật | Bản giả |
|---|---|---|---|---|
| PIR, cảm biến mưa, nút cảm ứng, công tắc từ MC-38 | `DigitalInput` (`read()` trả `bool`) | `BinarySensorDevice` (id, tên trường, mức tín hiệu, chống nhiễu 50 ms) | `Esp32DigitalIn(pin, pullup)` (`pullup` bật `INPUT_PULLUP`, dùng cho MC-38) | `FakeDigitalInput` |
| Còi, quạt hút bếp | `DigitalOutput` (đã có) | `SwitchDevice` (đã có) | `Esp32ToneOutput` (còi thụ động, phát xung) và `Esp32DigitalOut` (quạt hút, đã có) | `FakeDigitalOutput` (đã có) |
| Servo cửa, rèm, giàn phơi | `PwmOutput` (đã có) | `PositionDevice` (các vị trí có tên và góc); luật đổi góc ra duty nằm ở `core` | `Esp32Pwm(pin, kênh, 50, 16)` (đã có) | `FakePwm` (đã có) |
| Đèn WS2812 | `PixelStrip` (`setColor(r, g, b)` cho cả thanh) | `ColorLight` (luật bật tắt, màu, độ sáng nằm ở `core`) | `Ws2812Strip` (Adafruit NeoPixel) | `FakePixelStrip` |
| OLED | `TextDisplay` (`show(các dòng)`) | `DisplayDevice` (kiểm tra ký tự được nhận, số dòng, độ dài) | `Ssd1306Display` | `FakeTextDisplay` |
| RC522 | `CardReader` (`poll()` trả UID hoặc rỗng) | `CardReaderDevice` (chống lặp 2 giây) | `Rc522Reader` | `FakeCardReader` |
| Quang trở (module LM393) | `AnalogInput` (dùng lại) | `LightSensorDevice` (trung bình 8 mẫu, chuẩn hóa chiều AO, `dark` có trễ; luật ở `core/LightSensor.h`) | `Esp32AnalogIn` (dùng lại) | `FakeAnalogInput` (dùng lại) |
| MQ-2 | `AnalogInput` (`read()` trả 0 đến 4095) | `GasSensorDevice` (làm nóng, trung bình 8 mẫu, báo động có trễ; luật trễ nằm ở `core/Gas.h`, hàm trễ chung ở `core/Hysteresis.h`) | `Esp32AnalogIn` | `FakeAnalogInput` |

- `Device` có hai hàm với giá trị mặc định cũ: `virtual bool retainState() const { return true; }` và `virtual bool publishOnConnect() const { return true; }`. Hub dùng chúng khi gửi state. Chỉ `CardReaderDevice` trả `false`.
- **Quy tắc an toàn gas:** `GasSafety` (services) nối `GasSensorDevice` với hai `SwitchDevice` (còi, quạt hút) qua `onAlarmChange`. `SwitchDevice` có `setOn()` (bật tắt từ code trên board, báo `Hub` gửi state qua `tick`) và `lockOn()` (từ chối lệnh tắt). `Device` thêm `virtual bool runsOffline() const { return false; }`: `Hub::loop` vẫn tick các thiết bị này khi mất kết nối broker (không gửi state), nên quy tắc an toàn chạy được khi offline.
- **Nút cảm ứng điều khiển đèn trần:** `TouchToggle` (services) nối `BinarySensorDevice` (nút TTP223) với `DimmableLight` (đèn trần) qua `BinarySensorDevice::onChange`: chạm (giá trị đổi sang true sau chống nhiễu, không tính lần đọc đầu) thì gọi `DimmableLight::setOn(!isOn())`. `BinarySensorDevice` có `setRunsOffline()` (mặc định false) và `DimmableLight` luôn `runsOffline()` để nhận cờ "đổi từ board" qua `tick`, như `SwitchDevice`. Dùng lại `applyLightCommand`, không có luật mới ở `core`.
- Mỗi port mới có contract test (luật 6) và bản giả; mỗi lớp dịch vụ có test chạy trên laptop với bản giả.
- Thư viện bên thứ ba chỉ được include trong `adapters_esp32`. Thư viện nào chỉ board `comfort` hoặc `guard` cần thì khai báo `lib_deps` riêng cho env đó, và env còn lại loại file adapter tương ứng khỏi `build_src_filter` (giống `Dht22Sensor` cho `guard`).

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
