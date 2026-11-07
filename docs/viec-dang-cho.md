# Việc đang chờ

Cập nhật 07/11/2026. Xóa mục nào khi xong.

## Chờ quyết: PR của Biên sẽ đổi chân theo bảng mới như thế nào

Bảng chân mới đã duyệt (`docs/gpio.md`, PR #14). Code của #8, #9, #10 đã đổi chân. Ba PR của Biên chưa:

| PR | Việc | Cần đổi |
|---|---|---|
| #11 (issue #2) | Còi và quạt hút bếp | `exhaustFanRelay(16, ...)` thành 32, sửa comment. Còi GPIO27 giữ nguyên |
| #12 (issue #3) | PIR, mưa, nút cảm ứng | `rainInput(34)` thành 35, sửa comment. PIR 39 và nút cảm ứng 27 giữ nguyên |
| #13 (issue #6) | OLED | Không đổi (I2C GPIO21, 22) |

Tình trạng lúc ghi: mỗi PR 1 commit từ 02/11, CI xanh, chưa ai duyệt, chưa có bình luận. Chưa gửi gì cho Biên, chưa đổi người nhận issue.

Chủ dự án muốn **duyệt PR của Biên trước**, rồi mới quyết cách tiến hành. Các cách đã nêu:
1. Giữ PR của Biên, đẩy thêm commit đổi chân (và sửa phần thiếu) vào nhánh đó, chuyển issue #2, #3, #6 sang chủ dự án. Giữ commit và tên Biên. Cách được khuyên.
2. Đóng PR của Biên, mở PR mới từ commit của Biên.
3. Chỉ chuyển issue, chưa đụng PR.

Duyệt theo quy trình 4 bước trong `~/CLAUDE.md` (checklist từ spec, Haiku chạy kiểm tra cơ học, so với spec, đọc lại điểm quan trọng). Phát hiện lỗi thì báo chủ dự án trước, chủ dự án quyết có gửi cho Biên không.

## Thử phần cứng sau khi đổi chân

- Đo hàng V của board mở rộng đúng 3,3 V (jumper 3.3/5V) **trước khi** cắm RC522. Lót đế không dẫn điện dưới board mở rộng (bàn kim loại gây reset liên tục).
- Kiểm tra hàng S ở cụm dưới nằm ngoài cùng hay sát ESP32, và thứ tự chân thật của từng module (nhất là OLED: SCL, SDA). Sai thì sửa `docs/so-do-noi-chan.html`.
- #8: servo cửa guard GPIO4, giàn phơi comfort GPIO32 (rèm giữ GPIO13). Đã thử servo ở chân cũ, thử lại ở chân mới.
- #9: WS2812 GPIO33. Chưa hàn, chưa thử. Giữ level không quá 50 khi cấp từ USB.
- #10: RC522 GPIO16 (SDA), 17 (SCK), 5 (MOSI), 18 (MISO), 19 (RST). Đã đọc được thẻ ở chân cũ: thẻ trắng `319A0F16`, thẻ xanh `B3FF552D`. Còn lại: để thẻ yên không gửi lặp, nhấc ra quẹt lại có tin ngay, bấm EN không phát lại UID cũ, gửi lệnh vào `cmd` ra lỗi "chỉ đọc".

## Gộp PR

- Chưa gộp PR nào. Chỉ gộp khi chủ dự án nói rõ.
- Gộp #14 trước thì #8, #9, #10 rebase lên main (xung đột nhỏ ở các dòng chân trong `gpio.md`, `mqtt-topics.md`).
- Sau khi gộp thiết bị nào: sửa tiêu đề "chưa làm" trong `docs/mqtt-topics.md`, mục "Thiết bị sẽ thêm" trong `docs/architecture.md`, các tab trong `docs/giai-thich-code.html`.
