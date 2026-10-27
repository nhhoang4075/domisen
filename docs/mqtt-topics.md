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

## Đèn trần (#1, đã làm)

Topic `domisen/living/ceiling-light/cmd` (nhận) và `.../state` (gửi, retain). `level` từ 0 đến 100.

| Lệnh gửi | Kết quả |
|---|---|
| `{"on":true}` | Bật, dùng mức sáng đã nhớ (mặc định 100) |
| `{"on":false}` | Tắt, nhớ mức sáng cũ |
| `{"level":40}` | Đặt mức 40 và bật (nếu không kèm `on`) |
| `{"on":true,"level":80}` | Bật ở mức 80 |
| `{"level":0}` | Tắt, giữ mức cũ |

Board trả lời bằng `.../state`: `{"on":true,"level":80}`. Gửi lại mỗi lần board nối lại broker, nên người mới đăng ký thấy ngay trạng thái hiện tại.

Đấu dây: GPIO25, điện trở 220 ohm, chân dài (anode) của LED, chân ngắn (cathode) về GND. PWM 5 kHz, 10 bit, có bù gamma 2,2.

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

## Bản thử đầu tiên (firmware hiện tại)

- `domisen/node/guard/status`, `domisen/node/comfort/status`: `online` / `offline`
- `domisen/node/<guard|comfort>/uptime`: số giây, 5 giây một lần
- `domisen/node/<guard|comfort>/ping`: board nhận và in ra Serial
