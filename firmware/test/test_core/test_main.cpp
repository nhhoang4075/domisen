// Luật thuần của đèn: không cần phần cứng, không cần JSON.
#include <domisen/core/Light.h>
#include <unity.h>

using namespace domisen::core;

void setUp() {}
void tearDown() {}

void test_mac_dinh_tat_muc_100() {
  LightState s;
  TEST_ASSERT_FALSE(s.on);
  TEST_ASSERT_EQUAL(100, s.level);
}

void test_bat_dung_muc_da_nho() {
  LightState s = applyLightCommand({}, std::nullopt, 40);   // đặt 40, tự bật
  s = applyLightCommand(s, false, std::nullopt);            // tắt
  s = applyLightCommand(s, true, std::nullopt);             // bật lại
  TEST_ASSERT_TRUE(s.on);
  TEST_ASSERT_EQUAL(40, s.level);
}

void test_chi_gui_level_thi_bat() {
  LightState s = applyLightCommand({}, std::nullopt, 70);
  TEST_ASSERT_TRUE(s.on);
  TEST_ASSERT_EQUAL(70, s.level);
}

void test_level_0_tat_nhung_giu_muc_cu() {
  LightState s = applyLightCommand({}, true, 60);
  s = applyLightCommand(s, std::nullopt, 0);
  TEST_ASSERT_FALSE(s.on);
  TEST_ASSERT_EQUAL(60, s.level);
}

void test_on_ghi_ro_thang_level() {
  LightState s = applyLightCommand({}, false, 80);  // đặt 80 nhưng nói rõ tắt
  TEST_ASSERT_FALSE(s.on);
  TEST_ASSERT_EQUAL(80, s.level);
}

void test_level_ngoai_khoang_bi_cat() {
  TEST_ASSERT_EQUAL(100, applyLightCommand({}, std::nullopt, 500).level);
  TEST_ASSERT_FALSE(applyLightCommand({}, std::nullopt, -5).on);
}

void test_duty_gamma() {
  TEST_ASSERT_EQUAL_FLOAT(0.0f, lightDuty({false, 100}));
  TEST_ASSERT_EQUAL_FLOAT(1.0f, lightDuty({true, 100}));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2176f, lightDuty({true, 50}));  // 0,5 mũ 2,2
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_mac_dinh_tat_muc_100);
  RUN_TEST(test_bat_dung_muc_da_nho);
  RUN_TEST(test_chi_gui_level_thi_bat);
  RUN_TEST(test_level_0_tat_nhung_giu_muc_cu);
  RUN_TEST(test_on_ghi_ro_thang_level);
  RUN_TEST(test_level_ngoai_khoang_bi_cat);
  RUN_TEST(test_duty_gamma);
  return UNITY_END();
}
