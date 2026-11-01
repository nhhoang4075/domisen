# AGENTS.md: hướng dẫn cho người và coding agent làm việc trên Domisen

Domisen là mô hình nhà thông minh gồm 2 board ESP32 (`guard` và `comfort`) nói chuyện với một server qua MQTT. Hiện đã làm phần firmware; server và web làm sau.

## Đọc trước khi làm

1. `README.md`: cài đặt, chạy broker, nạp board.
2. `docs/architecture.md`: **luật kiến trúc, bắt buộc**. Lõi thuần, thiết bị không biết phần cứng.
3. `docs/mqtt-topics.md`: tên topic và nội dung tin của từng thiết bị. Đây là hợp đồng.
4. `docs/gpio.md`: chân GPIO và cách đấu dây.
5. `docs/giai-thich-code.html`: giải thích code và các linh kiện (mở bằng trình duyệt).

## Luật không được phá

- **Lõi thuần.** `core`, `protocol`, `services` không include Arduino, WiFi, PubSubClient hay adapter. `bash tools/check-purity.sh` phải sạch.
- **Một tên, một nguồn.** Topic chỉ ghép trong `protocol/Topics.h`. Không viết chuỗi `"domisen/..."` ở chỗ khác.
- **Thiết bị không biết phần cứng.** Chân GPIO, kênh LEDC, thư viện cảm biến chỉ xuất hiện trong `adapters_esp32` và `main.cpp` của board.
- **Mỗi port mới có bản giả và contract test.** Mỗi thiết bị mới có test chạy trên laptop (`pio test -e native`).
- **Thiếu cấu hình thì dừng**, không âm thầm chạy tiếp.
- **Không bao giờ commit bí mật.** `firmware/include/domisen_config.h` và `broker/passwd` đã bị gitignore; đừng ép thêm vào, đừng dán mật khẩu vào issue, PR hay log.
- Include theo đường dẫn tính từ `src/`: `#include <shared/core/Ports.h>`, không dùng `../`.

## Spec là nguồn sự thật

Code lệch `docs/mqtt-topics.md` hay `docs/architecture.md` là **lỗi của code**, kể cả khi code chạy được và test xanh.

- **Không sửa tài liệu cho khớp code.** Tài liệu chỉ đổi khi chủ dự án quyết định đổi thiết kế.
- **Thấy tài liệu chưa đúng hoặc chưa đủ** (mâu thuẫn, thiếu chi tiết, có cách tốt hơn): **dừng và báo ở issue** bằng một bình luận: tài liệu ghi gì, đề xuất đổi gì, vì sao. Chờ chủ dự án duyệt rồi mới làm. Không tự thay bằng thứ "đơn giản hơn".
- **Trước khi code**, liệt kê yêu cầu cụ thể trong issue và tài liệu (tên trường, giá trị, chân, thư viện, mã lỗi) và làm đúng như vậy.
- Chi tiết nhỏ không có trong tài liệu (tên hàm, tên biến, bố cục file) do người làm quyết định, miễn đúng kiến trúc.

## Viết issue giao việc

Issue **không thể** giải thích hết logic và API. Mỗi issue giao việc định hướng bằng đường dẫn rõ ràng, đáng tin, để người làm tự đọc:

- Có mục **"Thư viện và tài liệu cần đọc"** gồm: thư viện cần dùng (tên đầy đủ, **phiên bản ghim cứng**, ghi rõ chỉ board nào dùng), API (liên kết đến tài liệu chính thức của đúng hàm hoặc lớp cần dùng), và tài liệu để hiểu linh kiện (datasheet, hướng dẫn của nhà sản xuất).
- **Ưu tiên nguồn chính thống:** tài liệu Espressif, PlatformIO, kho mã và trang tài liệu của chính thư viện, datasheet của nhà sản xuất chip. Bài của cộng đồng chỉ dùng bổ sung và ghi rõ đó là bài cộng đồng.
- **Mở thử từng liên kết trước khi đăng.** Không ghi liên kết từ trí nhớ. Không có liên kết ổn định thì ghi thẳng "chưa có liên kết", không đoán đường dẫn.
- Nói rõ phần **chưa kiểm chứng** (ví dụ: phiên bản thư viện đã có trên registry nhưng chưa biên dịch với nền tảng đang ghim).
- Không chép API vào issue. Issue chỉ nói **việc cần đạt, giá trị cụ thể trong tài liệu dự án** (tên trường, chân, số), và đường dẫn đến nơi đọc cách làm.

## Công cụ

PlatformIO (xem README). Mọi lệnh chạy trong thư mục `firmware`:

| Lệnh | Việc |
|---|---|
| `pio test -e native` | Test lõi, protocol, services, contract trên laptop, không cần board |
| `bash tools/check-purity.sh` | Lõi không dính phần cứng |
| `pio run` | Biên dịch cả hai board (cần `include/domisen_config.h`, sao từ `domisen_config.example.h`, giá trị giả cũng được để biên dịch) |

Nạp lên board là việc của người có phần cứng. Người viết code không cần board: dùng bản giả để chứng minh code đúng, rồi ghi trong PR những gì cần thử trên board.

## Commit

Tiếng Việt, một dòng đầu ngắn nói rõ việc đã làm, ví dụ `Thêm cảm biến PIR living/motion`. Không thêm dòng `Co-Authored-By` của AI và không thêm dòng "Generated with..." vào PR.

## Nhánh và PR

- `main` luôn xanh và không ai push thẳng lên đó: mọi thay đổi đi qua PR.
- Tên nhánh `<loại>/<mô-tả-ngắn>`, chữ thường, nối bằng gạch ngang, ví dụ `feat/pir-motion`, `fix/servo-angle`.
- **Một PR làm một việc.** Thân PR ghi: việc đã làm, cách kiểm tra (lệnh và kết quả), những gì **cần thử trên board** (checklist cho chủ dự án), và `Closes #<số issue>`.
- Gộp bằng **squash merge**. Cập nhật nhánh bằng `git rebase origin/main`, không merge `main` vào nhánh. Nhiều PR cùng sửa `Ports.h`, `Device.h`, `main.cpp` thì PR nào merge sau tự rebase và giải quyết xung đột.
- PR được merge khi CI xanh và chủ dự án đã duyệt. Review theo ba mức: **sửa trước khi merge**, **nhỏ**, **để sau**. Việc nhỏ cũng sửa trong cùng PR, trừ khi nó phụ thuộc thứ chưa có.
- Đừng làm tiếp trên nhánh đã merge, mở nhánh mới từ `main`.

## Xong nghĩa là

- `pio test -e native`, `bash tools/check-purity.sh` và `pio run` đều xanh.
- Đã đối chiếu **từng yêu cầu** trong issue và `docs/mqtt-topics.md` với code. Giá trị trong test lấy từ tài liệu, không lấy từ code.
- Tài liệu liên quan đã cập nhật nếu hành vi thật đổi (và chỉ khi chủ dự án đã duyệt đổi).
- PR có checklist cần thử trên board.
