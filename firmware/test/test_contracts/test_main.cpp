// Contract của từng port: hành vi mà Hub và thiết bị dựa vào. Chạy cho mọi adapter chạy được trên laptop
// (hiện là bản giả). Adapter thật cần board nên chỉ kiểm tra được khi nạp lên thiết bị.
#include <shared/adapters_fake/FakeClimateSensor.h>
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

void test_fake_transport() { transportContract<adapters::fake::FakeTransport>(); }
void test_fake_pwm() { pwmContract<adapters::fake::FakePwm>(); }
void test_fake_climate() { climateContract<adapters::fake::FakeClimateSensor>(); }

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_fake_transport);
  RUN_TEST(test_fake_pwm);
  RUN_TEST(test_fake_climate);
  return UNITY_END();
}
