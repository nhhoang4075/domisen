# Bảng gán chân GPIO

Đây là nguồn sự thật khi viết firmware và khi đấu dây. Nếu đổi chân thì sửa file này trước, rồi mới sửa code.
Bảng gốc lấy từ `docs/plan/ke-hoach.html`, đã được đổi chân ngày 07/11/2026 (xem cuối file). Sơ đồ trực quan theo đúng thứ tự chân board mở rộng: `docs/so-do-noi-chan.html`.

## guard (ESP32 A: cửa, bếp, an ninh)

| # | Thiết bị | Chân | Ghi chú |
|---|---|---|---|
| 7 | Servo cửa SG90 | GPIO4 | Dây đỏ nối 5V adapter, không lấy từ ESP32 |
| 8 | OLED 0,96" (I2C) | GPIO21 SDA, GPIO22 SCL | |
| 9 | Relay quạt hút | GPIO32 | Quạt lấy điện 5V từ adapter qua tiếp điểm relay |
| 10 | Buzzer | GPIO27 | |
| 11 | RC522 (SPI) | GPIO16 SDA (SS), 17 SCK, 5 MOSI, 18 MISO, 19 RST | **Nguồn 3,3V, không cắm 5V.** Năm chân liền nhau, đúng thứ tự trên module. SPI chạy qua GPIO matrix nên chân tùy ý, tốc độ 4 MHz |
| 12 | Công tắc từ MC-38 | GPIO26 | Một dây vào chân, một dây vào GND, đặt `INPUT_PULLUP` trong code, không cần điện trở ngoài |
| 13 | MQ-2 (AO) | GPIO35 | Qua cầu phân áp vì MQ-2 xuất tới 5V |

## comfort (ESP32 B: phòng khách, phòng ngủ, ban công)

| # | Thiết bị | Chân | Ghi chú |
|---|---|---|---|
| 1 | Đèn trần (LED trắng) | GPIO25 | Qua 220 Ω, PWM chỉnh độ sáng |
| 2 | Đèn ngủ (LED vàng) | GPIO26 | Qua 220 Ω, PWM |
| 3 | Relay quạt phòng khách | GPIO16 | Quạt lấy điện 5V từ adapter qua relay. Module relay đang dùng chạy mức cao, xem `kFanRelayActiveLow` (đang là `false`) trong `comfort/main.cpp` |
| 4 | DHT22 (module 3 chân) | GPIO14 | Nguồn 3,3V. Đọc chậm nhất 2 giây một lần, firmware đọc 10 giây một lần |
| 5 | Quang trở (AO) | GPIO34 | ADC1, đọc được khi WiFi bật |
| 6 | PIR HC-SR501 | GPIO39 | Chân chỉ đọc |
| 14 | Servo rèm SG90 | GPIO13 | Dây đỏ nối 5V adapter |
| 15 | Vòng LED WS2812 | GPIO33 | Nguồn 5V từ adapter |
| 16 | Cảm biến mưa (DO) | GPIO35 | Chỉnh độ nhạy bằng núm trên module |
| 17 | Servo giàn phơi SG90 | GPIO32 | Dây đỏ nối 5V adapter |
| 19 | Nút cảm ứng TTP223 | GPIO27 | Nguồn 3,3V. Board tự xử lý, mất mạng vẫn dùng được |

## Cần nhớ

- GPIO34, 35, 36, 39 chỉ **đọc**, không có pull-up/pull-down nội. Không dùng làm đầu ra.
- Không dùng GPIO6 đến 11 (nối flash) và hạn chế GPIO0, 2, 12, 15 (ảnh hưởng lúc khởi động).
- GPIO2 là LED có sẵn trên board, dùng cho bản thử đầu tiên.
- Mọi servo, relay, quạt, WS2812 lấy nguồn 5V từ adapter, chung GND với ESP32.
- GPIO16 và 17 (RX2, TX2) chỉ dùng được trên ESP32-WROOM. Bản WROVER dùng hai chân này cho PSRAM.
- Board mở rộng: jumper 3.3/5V đặt ở **3,3V** để hàng V là 3,3V. Đo hàng V bằng đồng hồ vạn năng trước khi cắm RC522.
- Với servo và relay, rút dây V khỏi đầu cắm 3 chân rồi nối riêng vào 5V adapter. Cắm thẳng thì V chỉ có 3,3V.

## Lịch sử đổi chân

**07/11/2026, chủ dự án duyệt** (đề xuất trong `docs/so-do-noi-chan.html`). Mục tiêu: dây của cùng một thiết bị nằm sát nhau, đúng thứ tự chân trên module, ít bắt chéo, thiết bị cùng phòng nằm gần nhau. Với bảng cũ, guard có 11 chỗ dây bắt chéo, bảng mới còn 1 (SDA/SCL của OLED, do thứ tự chân module khác cổng I2C); comfort cả hai bảng đều 0.

| Thiết bị | Board | Cũ | Mới | Lý do |
|---|---|---|---|---|
| Servo cửa | guard | GPIO13 | GPIO4 | Gom cụm cửa vào (OLED, RC522, servo) |
| RC522 | guard | GPIO5, 18, 23, 19, 4 (SS, SCK, MOSI, MISO, RST) | GPIO16, 17, 5, 18, 19 | Năm chân liền nhau, đúng thứ tự module |
| Relay quạt hút | guard | GPIO16 | GPIO32 | Cạnh MQ-2 (cụm bếp), nhường GPIO16 cho RC522 |
| MC-38 | guard | GPIO34 | GPIO26 | Có pull-up nội, bỏ điện trở 10 kΩ |
| MQ-2 | guard | GPIO36 | GPIO35 | Cạnh relay quạt hút |
| Vòng LED WS2812 | comfort | GPIO32 | GPIO33 | Cạnh đèn trần, nhường GPIO32 cho servo giàn phơi |
| Servo giàn phơi | comfort | GPIO33 | GPIO32 | Cạnh cảm biến mưa (cụm ban công) |
| Cảm biến mưa | comfort | GPIO34 | GPIO35 | Cạnh servo giàn phơi |
| Quang trở | comfort | GPIO35 | GPIO34 | Cạnh PIR (hai cảm biến phòng khách) |
