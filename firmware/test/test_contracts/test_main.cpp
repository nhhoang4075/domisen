// Contract của từng port: hành vi mà Hub và thiết bị dựa vào. Chạy cho mọi adapter chạy được trên laptop
// (hiện là bản giả). Adapter thật cần board nên chỉ kiểm tra được khi nạp lên thiết bị.
#include <shared/adapters_fake/FakeCardReader.h>
#include <shared/adapters_fake/FakeClimateSensor.h>
#include <shared/adapters_fake/FakeDigitalOutput.h>
#include <shared/adapters_fake/FakeDigitalInput.h>
#include <shared/adapters_fake/FakePixelStrip.h>
#include <shared/adapters_fake/FakePwm.h>
#include <shared/adapters_fake/FakeTransport.h>
#include <unity.h>

using namespace domisen;

void setUp() {}
void tearDown() {}

// ---- Transport ----
template <typename T> void transportContract() {
  T t;
  int connects = 0;
  std::vector<std::string> received;
  t.onConnect([&] { connects++; });
  t.onMessage([&](const std::string& topic, const std::string&) { received.push_back(topic); });

  // chưa nối: publish phải trả false
  TEST_ASSERT_FALSE(t.publish("a/b", "x", false));
  t.subscribe("a/b");  // đăng ký trước khi nối được phép
  t.simulateConnect();
  TEST_ASSERT_TRUE(t.connected());
  TEST_ASSERT_EQUAL(1, connects);
  TEST_ASSERT_TRUE(t.publish("a/b", "x", true));

  // chỉ nhận topic đã đăng ký
  t.simulateMessage("a/b", "1");
  t.simulateMessage("khac/topic", "2");
  TEST_ASSERT_EQUAL(1, received.size());

  // đăng ký giữ qua lần nối lại, onConnect báo lại
  t.simulateDisconnect();
  TEST_ASSERT_FALSE(t.connected());
  t.simulateConnect();
  TEST_ASSERT_EQUAL(2, connects);
  t.simulateMessage("a/b", "3");
  TEST_ASSERT_EQUAL(2, received.size());

  // đăng ký trùng không nhân đôi
  t.subscribe("a/b");
  TEST_ASSERT_EQUAL(1, t.subscriptions.size());
}

// ---- PwmOutput ----
template <typename P> void pwmContract() {
  P p;
  p.write(0.5f);
  TEST_ASSERT_EQUAL_FLOAT(0.5f, p.last);
  p.write(-1.0f);  // ngoài khoảng thì bị cắt về 0..1
  TEST_ASSERT_EQUAL_FLOAT(0.0f, p.last);
  p.write(7.0f);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, p.last);
}

// ---- ClimateSensor ----
template <typename S> void climateContract() {
  S s;
  s.next = core::ClimateReading{27.5f, 62.0f};
  auto ok = s.read();
  TEST_ASSERT_TRUE(ok.has_value());
  TEST_ASSERT_EQUAL_FLOAT(27.5f, ok->temp);
  TEST_ASSERT_EQUAL_FLOAT(62.0f, ok->humidity);
  s.next = std::nullopt;  // đọc lỗi phải là nullopt, không phải NaN
  TEST_ASSERT_FALSE(s.read().has_value());
}

// ---- DigitalOutput ----
template <typename O> void digitalOutputContract() {
  O o;
  o.write(true);
  TEST_ASSERT_TRUE(o.last);
  o.write(false);
  TEST_ASSERT_FALSE(o.last);
}

// ---- DigitalInput ----
template <typename I> void digitalInputContract() {
  I input;
  input.value = true;
  TEST_ASSERT_TRUE(input.read());
  input.value = false;
  TEST_ASSERT_FALSE(input.read());
}

// ---- PixelStrip ----
template <typename S> void pixelStripContract() {
  S s;
  s.setColor(255, 180, 107);
  TEST_ASSERT_EQUAL_UINT8(255, s.last.r);
  TEST_ASSERT_EQUAL_UINT8(180, s.last.g);
  TEST_ASSERT_EQUAL_UINT8(107, s.last.b);
  s.setColor(0, 0, 0);  // tắt là đen, cả thanh
  TEST_ASSERT_EQUAL_UINT8(0, s.last.r);
  TEST_ASSERT_EQUAL_UINT8(0, s.last.g);
  TEST_ASSERT_EQUAL_UINT8(0, s.last.b);
  s.setColor(1, 2, 3);  // kênh nào ra kênh đó, không đảo thứ tự
  TEST_ASSERT_EQUAL_UINT8(1, s.last.r);
  TEST_ASSERT_EQUAL_UINT8(2, s.last.g);
  TEST_ASSERT_EQUAL_UINT8(3, s.last.b);
}

// ---- CardReader ----
template <typename R> void cardReaderContract(R& r) {
  r.push("A3F2197C");
  auto card = r.poll();
  TEST_ASSERT_TRUE(card.has_value());
  TEST_ASSERT_EQUAL_STRING("A3F2197C", card->c_str());  // hex chữ hoa, không dấu cách
  for (char c : *card) TEST_ASSERT_TRUE((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
  r.pushEmpty();
  TEST_ASSERT_FALSE(r.poll().has_value());  // không có thẻ mới là nullopt, không phải chuỗi rỗng
  TEST_ASSERT_FALSE(r.poll().has_value());  // hết hàng cũng là không có thẻ
}

void test_fake_transport() { transportContract<adapters::fake::FakeTransport>(); }
void test_fake_pwm() { pwmContract<adapters::fake::FakePwm>(); }
void test_fake_digital_output() { digitalOutputContract<adapters::fake::FakeDigitalOutput>(); }
void test_fake_digital_input() { digitalInputContract<adapters::fake::FakeDigitalInput>(); }
void test_fake_climate() { climateContract<adapters::fake::FakeClimateSensor>(); }
void test_fake_pixel_strip() { pixelStripContract<adapters::fake::FakePixelStrip>(); }
void test_fake_card_reader() {
  adapters::fake::FakeCardReader reader;
  cardReaderContract(reader);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_fake_transport);
  RUN_TEST(test_fake_pwm);
  RUN_TEST(test_fake_climate);
  RUN_TEST(test_fake_digital_output);
  RUN_TEST(test_fake_digital_input);
  RUN_TEST(test_fake_pixel_strip);
  RUN_TEST(test_fake_card_reader);
  return UNITY_END();
}
