// Tên topic và mã hóa JSON. Giá trị mong đợi lấy từ docs/mqtt-topics.md.
#include <shared/protocol/Error.h>
#include <shared/protocol/Light.h>
#include <shared/protocol/Topics.h>
#include <unity.h>

using namespace domisen::protocol;

void setUp() {}
void tearDown() {}

void test_topics() {
  TEST_ASSERT_EQUAL_STRING("domisen/living/ceiling-light/cmd", cmdTopic("living/ceiling-light").c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/living/ceiling-light/state", stateTopic("living/ceiling-light").c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/living/ceiling-light/error", errorTopic("living/ceiling-light").c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/node/guard/status", nodeStatusTopic("guard").c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/node/comfort/uptime", nodeUptimeTopic("comfort").c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/node/guard/ping", nodePingTopic("guard").c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/node/guard/pong", nodePongTopic("guard").c_str());
}

void test_giai_ma_lenh_day_du() {
  LightCommand c; std::string err;
  TEST_ASSERT_TRUE(decodeLightCommand("{\"on\":true,\"level\":80}", c, err));
  TEST_ASSERT_TRUE(c.on.has_value() && *c.on);
  TEST_ASSERT_TRUE(c.level.has_value());
  TEST_ASSERT_EQUAL(80, *c.level);
}

void test_giai_ma_lenh_mot_phan() {
  LightCommand c; std::string err;
  TEST_ASSERT_TRUE(decodeLightCommand("{\"level\":40}", c, err));
  TEST_ASSERT_FALSE(c.on.has_value());
  TEST_ASSERT_EQUAL(40, *c.level);
  TEST_ASSERT_TRUE(decodeLightCommand("{\"on\":false}", c, err));
  TEST_ASSERT_FALSE(*c.on);
  TEST_ASSERT_FALSE(c.level.has_value());
}

void test_lenh_sai_bi_tu_choi() {
  LightCommand c; std::string err;
  TEST_ASSERT_FALSE(decodeLightCommand("khong phai json", c, err));
  TEST_ASSERT_FALSE(decodeLightCommand("[1,2]", c, err));
  TEST_ASSERT_FALSE(decodeLightCommand("{}", c, err));
  TEST_ASSERT_FALSE(decodeLightCommand("{\"on\":\"yes\"}", c, err));
  TEST_ASSERT_FALSE(decodeLightCommand("{\"level\":\"cao\"}", c, err));
  TEST_ASSERT_FALSE(err.empty());
}

void test_ma_hoa_trang_thai() {
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"level\":80}", encodeLightState(true, 80).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":100}", encodeLightState(false, 100).c_str());
}

void test_ma_hoa_loi() {
  TEST_ASSERT_EQUAL_STRING("{\"error\":\"hỏng\"}", encodeError("hỏng").c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_topics);
  RUN_TEST(test_giai_ma_lenh_day_du);
  RUN_TEST(test_giai_ma_lenh_mot_phan);
  RUN_TEST(test_lenh_sai_bi_tu_choi);
  RUN_TEST(test_ma_hoa_trang_thai);
  RUN_TEST(test_ma_hoa_loi);
  return UNITY_END();
}
