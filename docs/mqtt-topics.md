# Quy ước MQTT

Mọi topic bắt đầu bằng `domisen/`. Chỉ dùng chữ thường không dấu, kebab-case. Nội dung là JSON.

## Hình dạng topic

| Topic | Hướng | Retain | Ý nghĩa |
|---|---|---|---|
| `domisen/<phòng>/<thiết-bị>/state` | board → server | có | Trạng thái hiện tại |
| `domisen/<phòng>/<thiết-bị>/cmd` | server → board | không | Lệnh điều khiển |
| `domisen/node/<board>/status` | board → server | có | `online` / `offline` (last will) |
| `domisen/mode/set` | web → server | không | Đổi chế độ: `home`, `away`, `sleep` |
| `domisen/mode/state` | server → mọi nơi | có | Chế độ hiện tại |

Topic thiết bị theo **phòng và thiết bị**, không theo board. Riêng board ("node") có nhóm `domisen/node/<tên>/...`, server đăng ký `domisen/node/+/status` là thấy cả hai. Dời thiết bị sang board khác thì topic không đổi.

## Ví dụ nội dung

```
domisen/living/ceiling-light/cmd    {"on":true,"level":80}
domisen/living/climate/state        {"temp":29.4,"humidity":71}
domisen/kitchen/gas/state           {"ppm_raw":1830,"alarm":true}
domisen/entrance/door/cmd           {"open":true}
```

## Đèn chỉnh độ sáng (đã làm: #1 đèn trần, #2 đèn ngủ)

Áp dụng cho `living/ceiling-light` (GPIO25) và `bedroom/night-light` (GPIO26). Ví dụ `domisen/living/ceiling-light/cmd` (nhận) và `.../state` (gửi, retain). `level` từ 0 đến 100.

| Lệnh gửi | Kết quả |
|---|---|
| `{"on":true}` | Bật, dùng mức sáng đã nhớ (mặc định 100) |
| `{"on":false}` | Tắt, nhớ mức sáng cũ |
| `{"level":40}` | Đặt mức 40 và bật (nếu không kèm `on`) |
| `{"on":true,"level":80}` | Bật ở mức 80 |
| `{"level":0}` | Tắt, giữ mức cũ |

Board trả lời bằng `.../state`: `{"on":true,"level":80}`. Gửi lại mỗi lần board nối lại broker, nên người mới đăng ký thấy ngay trạng thái hiện tại.

Đấu dây: GPIO25, điện trở 220 ohm, chân dài (anode) của LED, chân ngắn (cathode) về GND. PWM 5 kHz, 10 bit, có bù gamma 2,2.

## Nhiệt độ và độ ẩm (đã làm: #4 DHT22)

Topic `domisen/living/climate/state` (retain), board gửi mỗi 10 giây và mỗi lần nối lại broker. Không có lệnh: gửi vào `.../cmd` sẽ nhận lỗi "chỉ đọc".

| Trường | Ý nghĩa |
|---|---|
| `temp` | Nhiệt độ, độ C, làm tròn 1 chữ số thập phân |
| `humidity` | Độ ẩm tương đối, %, làm tròn 1 chữ số thập phân |

Ví dụ `{"temp":27.5,"humidity":62}`. Cảm biến đọc lỗi (dây lỏng, chưa đo được lần nào) thì gửi `{"temp":null,"humidity":null}`.

Đấu dây: DHT22 loại module 3 chân. Chân S (DATA) vào GPIO14, V vào 3V3, G vào GND. Trên board mở rộng cắm đúng bộ G-V-S của GPIO14.

## Quạt phòng khách chỉnh tốc độ (#3, đổi từ relay sang MOSFET ngày 07/11/2026)

Chủ dự án quyết định: quạt phòng khách (`living/fan`) chỉnh được tốc độ bằng PWM qua module MOSFET, thay cho relay bật tắt. Relay cũ bỏ. Vẫn dùng GPIO16.

Lệnh và state **giống đèn chỉnh độ sáng** (xem mục trên): `on` và `level` từ 0 đến 100.

| Lệnh gửi | Kết quả |
|---|---|
| `{"on":true}` | Bật, dùng tốc độ đã nhớ (mặc định 100) |
| `{"on":false}` | Tắt, nhớ tốc độ cũ |
| `{"level":40}` | Đặt tốc độ 40 và bật (nếu không kèm `on`) |
| `{"level":0}` | Tắt, giữ tốc độ cũ |

State `{"on":true,"level":40}` (retain, gửi lại mỗi lần nối lại broker). Lệnh cũ `{"on":true}` và `{"on":false}` vẫn chạy. Lúc bật board quạt luôn tắt.

Khác đèn ở cách đổi `level` ra duty PWM (luật nằm ở `core/Fan.h`):

- Không bù gamma, và quạt nhỏ không quay khi duty thấp, nên level 1 đến 100 đi tuyến tính từ duty tối thiểu đến 1,0: `duty = kFanMinDuty + (1 - kFanMinDuty) * level / 100`. Tắt là duty 0.
- `kFanMinDuty` (`comfort/main.cpp`) mặc định 0,3, chỉnh khi thử trên quạt thật (QGDC4010HSL). Đã đo: không có kick-start thì từ đứng yên phải duty 0,314 trở lên quạt mới quay (0,307 thì không). Có kick-start 300 ms thì phải từ level 8 (duty 0,31 khi min 0,25) quạt mới chạy tốt. Tăng kick-start lên 500 ms thì hạ được min xuống 0,3, level 1 (duty 0,307) khởi động từ đứng yên và chạy tốt: quạt cần thời gian đạt vòng quay trước khi hạ ga. Thay quạt khác thì đo lại.
- **Kick-start (chạy hết ga lúc khởi động):** quạt đứng yên cần duty cao hơn lúc đã quay (đã thấy khi thử: đang quay thì level 1 vẫn quay, đứng yên rồi đặt level 1 thì không tự quay). Nên khi quạt đi từ đứng yên (duty đang ra là 0) lên một duty thấp hơn 1,0, board cho chạy **hết ga** `kFanKickMs` (mặc định 500 ms, `comfort/main.cpp`) rồi mới hạ về duty đã đặt. Đang quay mà đổi mức thì không kick. Đổi mức trong lúc đang kick chỉ đổi mức đích, không kéo dài thời gian kick. Tắt trong lúc kick thì dừng ngay. Luật `fanNeedsKick` nằm ở `core/Fan.h`; thời gian do dịch vụ `SpeedFan` quản lý (qua `tick` của Hub, nên chỉ chạy khi board đang nối broker).

Phần cứng: module MOSFET **LR7843** (IRLR7843, logic-level nên 3,3 V mở đủ) có opto **EL817** cách ly giữa phía tín hiệu và phía tải. PWM do LEDC kênh 4 (cặp 4 và 5, tách khỏi đèn kênh 0, 1 và servo kênh 2, 3), 10 bit, tần số là hằng số `kFanPwmHz` (mặc định 5000 Hz). EL817 chậm (mỗi cạnh vài micro giây đến hơn chục micro giây) nên **không dùng tần số 25 kHz** thường gặp ở quạt máy tính; nếu quạt rít thì thử các tần số khác trên module thật rồi chốt.

Đấu dây (nhãn đọc ở mặt dưới module; đặt module mặt có chip hướng lên, phía tín hiệu bên trái, chữ `GND` ở trên chữ `PWM`):

- Phía tín hiệu (hai lỗ bên trái, hàn hàng chân đực 2 chân): `PWM` vào GPIO16, `GND` vào GND của ESP32.
- Phía tải (ba lỗ bên phải, hàn cọc vít xanh dương 3 chân). Nhìn từ mặt có chip, từ trên xuống: `-`, `LOAD`, `+`. Nhãn ở mặt dưới đọc ngược lại vì lật bo.
  - `+`: nguồn 5V từ adapter, và dây dương của quạt (hai dây chung một cọc).
  - `LOAD`: dây âm của quạt. Đây là đầu được đóng ngắt.
  - `-`: GND của adapter.
- Phía tín hiệu cách ly với phía tải bằng opto, nên không bắt buộc nối GND của ESP32 với GND của adapter.

Tự bật quạt theo nhiệt độ là luật của **server** (đọc `living/climate/state`, gửi lệnh vào `living/fan/cmd`), không nằm trong board. Hiện chỉ điều khiển tay, ví dụ bằng MQTTX.

## Thiết bị bật tắt (relay, đã làm: #9 quạt hút bếp)

Áp dụng cho `kitchen/exhaust-fan` (relay GPIO32). Chỉ có `on`, không có mức.

| Lệnh gửi | Kết quả |
|---|---|
| `{"on":true}` | Bật |
| `{"on":false}` | Tắt |

Thiếu `on`, hoặc `on` không phải `true`/`false` thì giữ nguyên trạng thái và báo lỗi ở `.../error`. Board trả lời bằng `.../state`: `{"on":true}` (retain, gửi lại mỗi lần nối lại broker). Lúc bật board, thiết bị luôn tắt.

Đấu dây: module relay 1 kênh 5V. IN vào chân điều khiển, VCC vào 5V, GND chung. Quạt 5V đi qua tiếp điểm COM và NO của relay, nguồn quạt từ adapter 5V. Module relay hay chạy mức thấp (IN xuống 0 V thì relay hút), chỉnh bằng hằng số `ActiveLow` trong `main.cpp`. Module đang dùng chạy mức cao (hút khi IN lên 5 V) nên hằng số này là `false`.

## Cảm biến hai trạng thái (chưa làm: PIR, cảm biến mưa, nút cảm ứng)

Cùng một kiểu thiết bị, chỉ khác id, chân, tên trường và mức tín hiệu. Chỉ gửi `state` (retain), không có lệnh: gửi vào `.../cmd` nhận lỗi "chỉ đọc" như DHT22.

| # | Id | Board, chân | Trường | Mức tín hiệu |
|---|---|---|---|---|
| 6 | `living/motion` (PIR HC-SR501) | comfort, GPIO39 | `motion` | HIGH là có chuyển động |
| 16 | `balcony/rain` (cảm biến mưa, chân DO) | comfort, GPIO35 | `raining` | LOW là đang mưa (module kéo DO xuống khi ướt) |
| 19 | `living/wall-button` (TTP223) | comfort, GPIO27 | `pressed` | HIGH là đang chạm |

Ví dụ `domisen/living/motion/state`: `{"motion":true}`.
- Giá trị chỉ đổi khi tín hiệu đã **ổn định** một khoảng thời gian (chống nhiễu): mặc định **50 ms** (PIR, nút cảm ứng). Riêng cảm biến mưa là **1 giây** (`kRainDebounceMs` trong `comfort/main.cpp`): chip so sánh LM393 trên module không có độ trễ nên chân DO nhấp nháy khi nhỏ nước, mà mưa là hiện tượng chậm. Chủ dự án quyết định ngày 07/11/2026, sau khi thử trên module thật.
- Gửi ngay khi đổi, và gửi lại khi nối lại broker. Lần đọc đầu tiên sau khi khởi động được nhận ngay (không chờ 50 ms) và gửi đi. Trước lần đọc đầu tiên, state là `{"motion":null}` (cùng cách với DHT22 khi chưa đo được).
- GPIO35 và 39 chỉ đọc, không có điện trở kéo nội: module phải tự kéo mức (PIR và DO của cảm biến mưa đều tự kéo). Chân chưa nối module sẽ nhấp nháy khi có tay lại gần, đó không phải lỗi.
- Mức tín hiệu (HIGH hay LOW là "bật") là hằng số của từng thiết bị trong `main.cpp`; chủ dự án kiểm tra trên module thật.
- PIR cần khoảng 1 phút sau khi cấp điện mới ổn định, và tự giữ mức HIGH vài giây theo núm trên module. Đó là việc của module, không xử lý trong firmware. Jumper của HC-SR501 để ở **H** (kích lặp lại: còn chuyển động thì OUT giữ mức cao); ở L (kích một lần) OUT tự hạ sau thời gian giữ dù vẫn còn chuyển động.

## Còi và quạt hút (chưa làm: #10 còi, #9 quạt hút bếp)

Dùng lại thiết bị bật tắt của quạt phòng khách: lệnh `{"on":true}` hoặc `{"on":false}`, state `{"on":true}` (retain), khởi động ở trạng thái tắt.

| # | Id | Board, chân | Phần cứng |
|---|---|---|---|
| 10 | `house/alarm` | guard, GPIO27 | Module buzzer **thụ động** 5V, chân VCC vào 5V. Chân I/O kéo lên VCC, kích ở mức thấp |
| 9 | `kitchen/exhaust-fan` | guard, GPIO32 | Relay như quạt phòng khách |

Relay quạt hút có thể chạy mức cao hoặc mức thấp: là hằng số `kExhaustFanActiveLow` trong `guard/main.cpp` (mặc định `false`: hút khi chân lên mức cao), chủ dự án chỉnh khi thử trên board.
Còi là module **thụ động**: cấp mức cao hoặc thấp liên tục chỉ nghe tiếng tách lúc chuyển mức, nên khi bật phải đưa **xung 50 %** ra chân (3000 Hz, hằng số `kAlarmToneHz`, đã thử quét 1000 đến 5000 Hz trên module thật và chọn 3000 Hz), khi tắt **thả chân về đầu vào** (trở kháng cao). Module có điện trở kéo I/O lên 5 V và kích ở mức thấp: giữ mức thấp thì có dòng chạy liên tục qua còi, còn xuất mức cao từ ESP32 chỉ có 3,3 V nên transistor vẫn hé bật và còi nóng (đã thấy khi thử trên board). Thả chân thì điện trở kéo lên đưa I/O lên đủ 5 V, transistor tắt hẳn. Làm bằng adapter `Esp32ToneOutput` cài đặt `DigitalOutput`, nên thiết bị vẫn là `SwitchDevice` với lệnh `{"on":true}` như cũ. Kênh LEDC của còi phải khác cặp với servo (mỗi cặp kênh dùng chung một bộ định thời, tức chung tần số).
Việc tự bật còi và quạt hút khi có gas (an toàn, chạy ngay trên board) làm sau khi có cảm biến MQ-2.

## Servo: cửa, rèm, giàn phơi (chưa làm)

Servo SG90 góc 0 đến 180 độ. Mỗi thiết bị có **hai vị trí có tên**, mỗi vị trí ứng với một góc.

| # | Id | Board, chân | Vị trí (tên: góc ban đầu) |
|---|---|---|---|
| 7 | `entrance/door` | guard, GPIO4 | `closed`: 0, `open`: 90 |
| 14 | `living/curtain` | comfort, GPIO13 | `open`: 0, `closed`: 90 |
| 17 | `balcony/clothesline` | comfort, GPIO32 | `in`: 0, `out`: 90 |

Lệnh `{"position":"open"}`, state `{"position":"open"}` (retain). Tên vị trí không có trong bảng thì báo lỗi và giữ nguyên. Khởi động ở vị trí **đầu tiên** của hàng (`closed`, `open`, `in`).
Góc là hằng số trong `main.cpp`, chủ dự án chỉnh khi lắp cơ khí. Servo cần 5V từ adapter, không lấy từ ESP32 (xem `docs/gpio.md`).
Điều khiển bằng PWM 50 Hz, xung 0,5 ms (0 độ) đến 2,4 ms (180 độ), bằng cổng `PwmOutput` và `Esp32Pwm(pin, kênh, 50, 16)` đã có, **không thêm thư viện servo**. Mỗi servo một kênh LEDC riêng (kênh 0 và 1 đã dùng cho đèn).

## Đèn màu WS2812 (chưa làm: #15 đèn tâm trạng)

`living/mood-light`, comfort GPIO33, thanh 8 bóng. Cả thanh luôn cùng một màu.

| Lệnh gửi | Kết quả |
|---|---|
| `{"on":true}` | Bật, dùng màu và mức đã nhớ (mặc định trắng ấm `#FFB46B`, mức 100) |
| `{"on":false}` | Tắt, nhớ màu và mức cũ |
| `{"color":"#FF0000"}` | Đổi màu và bật nếu lệnh không kèm `on` |
| `{"level":40}` | Đổi độ sáng 0 đến 100, và bật nếu lệnh không kèm `on` (0 là tắt, giữ mức cũ) |

State `{"on":true,"color":"#FF0000","level":40}` (retain). `color` là chuỗi `#RRGGBB` (chữ hoa hoặc thường, trả về chữ hoa). Sai định dạng thì báo lỗi và giữ nguyên.
Độ sáng nhân thẳng vào từng kênh màu (không bù gamma). Lệnh gửi về qua thư viện `Adafruit NeoPixel` đặt trong `adapters_esp32`.
Điện: 5V từ adapter. Thử trên USB thì chủ dự án giữ mức tối đa 50.

## Màn hình OLED (chưa làm: #8)

`entrance/display`, guard, I2C: GPIO21 SDA, GPIO22 SCL, SSD1306 128x64, địa chỉ 0x3C. Lệnh `{"text":"Xin chao"}`, state `{"text":"Xin chao"}` (retain).
- Tối đa 4 dòng, các dòng ngăn bằng `\n`, mỗi dòng tối đa 21 ký tự (phông chữ cỡ 1). Chỉ nhận ASCII in được (khoảng trắng đến `~`): tiếng Việt có dấu bị từ chối với lỗi rõ ràng, giữ nguyên màn hình.
- Quá 4 dòng hoặc dòng quá dài: báo lỗi, giữ nguyên.
- Khởi động: màn hình trống, state `{"text":""}`.
- Thư viện `Adafruit SSD1306` và `Adafruit GFX`, chỉ nằm trong `adapters_esp32`.

## Đầu đọc thẻ RC522 (chưa làm: #11)

`entrance/card-reader`, guard, SPI: GPIO16 SDA (SS), GPIO17 SCK, GPIO5 MOSI, GPIO18 MISO, GPIO19 RST (SPI chạy qua GPIO matrix, xem `docs/gpio.md`). **Nguồn 3,3V, không cắm 5V.** Thư viện `MFRC522`.
Khi quẹt thẻ: gửi `domisen/entrance/card-reader/state` là `{"uid":"A3F2197C"}` (UID hex **chữ hoa**, không dấu cách), **không retain**.
- Đây là **sự kiện**, không phải trạng thái: không gửi lại khi nối lại broker (nếu không cửa sẽ tự mở lại khi mất rồi có mạng).
- Cùng một thẻ giữ trên đầu đọc không gửi lặp: chỉ gửi lại sau khi thẻ rời đi, hoặc sau 2 giây.
- Không có lệnh: gửi vào `.../cmd` nhận lỗi "chỉ đọc".
- `Device` cần thêm hai hàm có mặc định cũ, để cảm biến sự kiện báo cho Hub biết: `retainState()` (mặc định `true`) và `publishOnConnect()` (mặc định `true`). Thiết bị này trả `false` cho cả hai.

## Danh sách thiết bị (đề xuất, sửa nếu sai tên phòng)

| # | Topic gốc | Board |
|---|---|---|
| 1 | `living/ceiling-light` | comfort |
| 2 | `bedroom/night-light` | comfort |
| 3 | `living/fan` | comfort |
| 4 | `living/climate` (DHT22) | comfort |
| 5 | `living/light-sensor` | comfort |
| 6 | `living/motion` | comfort |
| 7 | `entrance/door` | guard |
| 8 | `entrance/display` | guard |
| 9 | `kitchen/exhaust-fan` | guard |
| 10 | `house/alarm` (buzzer) | guard |
| 11 | `entrance/card-reader` | guard |
| 12 | `bedroom/window` | guard |
| 13 | `kitchen/gas` | guard |
| 14 | `living/curtain` | comfort |
| 15 | `living/mood-light` | comfort |
| 16 | `balcony/rain` | comfort |
| 17 | `balcony/clothesline` | comfort |
| 19 | `living/wall-button` | comfort |

## Topic của board (node)

| Topic | Hướng | Retain | Nội dung |
|---|---|---|---|
| `domisen/node/<board>/status` | board → server | có | `online` (Hub gửi khi nối), `offline` (broker gửi khi board mất kết nối) |
| `domisen/node/<board>/uptime` | board → server | không | Số giây từ lúc bật, 5 giây một lần |
| `domisen/node/<board>/ping` | → board | không | Nội dung bất kỳ |
| `domisen/node/<board>/pong` | board → | không | Board trả lại đúng nội dung của ping |

## Lỗi lệnh

Thiết bị không hiểu lệnh thì **giữ nguyên trạng thái** và gửi `domisen/<phòng>/<thiết-bị>/error` (không retain):
`{"error":"level phải là số nguyên từ 0 đến 100"}`. Xem được ngay trong MQTTX, không cần mở Serial.
