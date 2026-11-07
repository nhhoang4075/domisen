#pragma once
#include <MFRC522.h>
#include <shared/core/Ports.h>

namespace domisen::adapters::esp32 {

// Đầu đọc thẻ RC522 qua SPI, dùng thư viện MFRC522. Nguồn 3,3V, không cắm 5V.
class Rc522Reader : public core::CardReader {
 public:
  Rc522Reader(int ssPin, int rstPin, int sckPin, int misoPin, int mosiPin);
  void begin();
  std::optional<std::string> poll() override;

 private:
  int ssPin_, sckPin_, misoPin_, mosiPin_;
  MFRC522 mfrc_;
};

}  // namespace domisen::adapters::esp32
