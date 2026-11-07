#include <SPI.h>
#include <shared/adapters_esp32/Rc522Reader.h>

namespace domisen::adapters::esp32 {

Rc522Reader::Rc522Reader(int ssPin, int rstPin, int sckPin, int misoPin, int mosiPin)
    : ssPin_(ssPin), sckPin_(sckPin), misoPin_(misoPin), mosiPin_(mosiPin), mfrc_(ssPin, rstPin) {}

void Rc522Reader::begin() {
  SPI.begin(sckPin_, misoPin_, mosiPin_, ssPin_);
  mfrc_.PCD_Init();
}

std::optional<std::string> Rc522Reader::poll() {
  if (!mfrc_.PICC_IsNewCardPresent() || !mfrc_.PICC_ReadCardSerial()) return std::nullopt;
  static const char kHex[] = "0123456789ABCDEF";
  std::string uid;
  for (byte i = 0; i < mfrc_.uid.size; i++) {
    uid += kHex[mfrc_.uid.uidByte[i] >> 4];
    uid += kHex[mfrc_.uid.uidByte[i] & 0x0F];
  }
  mfrc_.PICC_HaltA();  // cho thẻ nghỉ, để thẻ còn nằm trên đầu đọc không bị đọc lại ngay
  mfrc_.PCD_StopCrypto1();
  return uid;
}

}  // namespace domisen::adapters::esp32
