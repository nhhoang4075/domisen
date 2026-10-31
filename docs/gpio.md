# Bảng gán chân GPIO

Lấy từ bảng đi dây trong `docs/plan/ke-hoach.html`. Đây là nguồn sự thật khi viết firmware và khi đấu dây.
Nếu đổi chân thì sửa file này trước, rồi mới sửa code.

## guard (ESP32 A: cửa, bếp, an ninh)

| # | Thiết bị | Chân | Ghi chú |
|---|---|---|---|
| 7 | Servo cửa SG90 | GPIO13 | Dây đỏ nối 5V adapter, không lấy từ ESP32 |
| 8 | OLED 0,96" (I2C) | GPIO21 SDA, GPIO22 SCL | |
| 9 | Relay quạt hút | GPIO16 | Quạt lấy điện 5V từ adapter qua tiếp điểm relay |
| 10 | Buzzer | GPIO27 | |
| 11 | RC522 (SPI) | GPIO5 SDA, 18 SCK, 23 MOSI, 19 MISO, 4 RST | **Nguồn 3,3V, không cắm 5V** |
| 12 | Công tắc từ MC-38 | GPIO34 | Thêm điện trở 10 kΩ kéo lên 3V3 (chân này không có pull-up nội) |
| 13 | MQ-2 (AO) | GPIO36 | Qua cầu phân áp vì MQ-2 xuất tới 5V |

## comfort (ESP32 B: phòng khách, phòng ngủ, ban công)

| # | Thiết bị | Chân | Ghi chú |
|---|---|---|---|
| 1 | Đèn trần (LED trắng) | GPIO25 | Qua 220 Ω, PWM chỉnh độ sáng |
| 2 | Đèn ngủ (LED vàng) | GPIO26 | Qua 220 Ω, PWM |
| 3 | Relay quạt phòng khách | GPIO16 | Quạt lấy điện 5V từ adapter qua relay |
| 4 | DHT22 | GPIO14 | |
| 5 | Quang trở (AO) | GPIO35 | ADC1, đọc được khi WiFi bật |
| 6 | PIR HC-SR501 | GPIO39 | Chân chỉ đọc |
| 14 | Servo rèm SG90 | GPIO13 | Dây đỏ nối 5V adapter |
| 15 | Vòng LED WS2812 | GPIO32 | Nguồn 5V từ adapter |
| 16 | Cảm biến mưa (DO) | GPIO34 | Chỉnh độ nhạy bằng núm trên module |
| 17 | Servo giàn phơi SG90 | GPIO33 | Dây đỏ nối 5V adapter |
| 19 | Nút cảm ứng TTP223 | GPIO27 | Nguồn 3,3V. Board tự xử lý, mất mạng vẫn dùng được |

## Cần nhớ

- GPIO34, 35, 36, 39 chỉ **đọc**, không có pull-up/pull-down nội. Không dùng làm đầu ra.
- Không dùng GPIO6 đến 11 (nối flash) và hạn chế GPIO0, 2, 12, 15 (ảnh hưởng lúc khởi động).
- GPIO2 là LED có sẵn trên board, dùng cho bản thử đầu tiên.
- Mọi servo, relay, quạt, WS2812 lấy nguồn 5V từ adapter, chung GND với ESP32.
