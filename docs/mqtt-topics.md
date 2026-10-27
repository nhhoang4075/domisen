# Quy ước MQTT

Mọi topic bắt đầu bằng `domisen/`. Chỉ dùng chữ thường không dấu, kebab-case. Nội dung là JSON.

## Hình dạng topic

| Topic | Hướng | Retain | Ý nghĩa |
|---|---|---|---|
| `domisen/<phòng>/<thiết-bị>/state` | board → server | có | Trạng thái hiện tại |
| `domisen/<phòng>/<thiết-bị>/cmd` | server → board | không | Lệnh điều khiển |
| `domisen/<board>/status` | board → server | có | `online` / `offline` (last will) |
| `domisen/mode/set` | web → server | không | Đổi chế độ: `home`, `away`, `sleep` |
| `domisen/mode/state` | server → mọi nơi | có | Chế độ hiện tại |

Topic theo **phòng và thiết bị**, không theo board. Dời thiết bị sang board khác thì topic không đổi.

## Ví dụ nội dung

```
domisen/living/ceiling-light/cmd    {"on":true,"level":80}
domisen/living/climate/state        {"temp":29.4,"humidity":71}
domisen/kitchen/gas/state           {"ppm_raw":1830,"alarm":true}
domisen/entrance/door/cmd           {"open":true}
```

## Danh sách thiết bị (đề xuất, sửa nếu sai tên phòng)

| # | Topic gốc | Board |
|---|---|---|
| 1 | `living/ceiling-light` | esp32-b |
| 2 | `bedroom/night-light` | esp32-b |
| 3 | `living/fan` | esp32-b |
| 4 | `living/climate` (DHT22) | esp32-b |
| 5 | `living/light-sensor` | esp32-b |
| 6 | `living/motion` | esp32-b |
| 7 | `entrance/door` | esp32-a |
| 8 | `entrance/display` | esp32-a |
| 9 | `kitchen/exhaust-fan` | esp32-a |
| 10 | `house/alarm` (buzzer) | esp32-a |
| 11 | `entrance/card-reader` | esp32-a |
| 12 | `bedroom/window` | esp32-a |
| 13 | `kitchen/gas` | esp32-a |
| 14 | `living/curtain` | esp32-b |
| 15 | `living/mood-light` | esp32-b |
| 16 | `balcony/rain` | esp32-b |
| 17 | `balcony/clothesline` | esp32-b |
| 19 | `living/wall-button` | esp32-b |

## Bản thử đầu tiên (firmware hiện tại)

- `domisen/esp32-a/status`, `domisen/esp32-b/status`: `online` / `offline`
- `domisen/esp32-<a|b>/uptime`: số giây, 5 giây một lần
- `domisen/esp32-<a|b>/ping`: board nhận và in ra Serial
