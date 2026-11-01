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

## Thiết bị bật tắt (đã làm: #3 quạt phòng khách)

Áp dụng cho `living/fan` (relay GPIO16). Sau này `kitchen/exhaust-fan` dùng cùng cách. Cả hai chỉ có `on`, không có mức.

| Lệnh gửi | Kết quả |
|---|---|
| `{"on":true}` | Bật |
| `{"on":false}` | Tắt |

Thiếu `on`, hoặc `on` không phải `true`/`false` thì giữ nguyên trạng thái và báo lỗi ở `.../error`. Board trả lời bằng `.../state`: `{"on":true}` (retain, gửi lại mỗi lần nối lại broker). Lúc bật board, quạt luôn tắt.

Tự bật quạt theo nhiệt độ là luật của **server** (đọc `living/climate/state`, gửi lệnh vào `living/fan/cmd`), không nằm trong board. Hiện chỉ điều khiển tay, ví dụ bằng MQTTX.

Đấu dây: module relay 1 kênh 5V. IN vào GPIO16, VCC vào 5V, GND chung. Quạt 5V đi qua tiếp điểm COM và NO của relay, nguồn quạt từ adapter 5V. Module relay hay chạy mức thấp (IN xuống 0 V thì relay hút), chỉnh bằng hằng số `kFanRelayActiveLow` trong `comfort/main.cpp`. Module đang dùng chạy mức cao (hút khi IN lên 5 V) nên hằng số này là `false`.

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
