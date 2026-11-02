// Luật thuần của đèn: không cần phần cứng, không cần JSON.
#include <shared/core/Light.h>
#include <shared/core/DisplayText.h>
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

void test_display_text_chap_nhan_mot_dong_va_bon_dong() {
  std::vector<std::string> lines;
  std::string error;
  TEST_ASSERT_TRUE(splitDisplayText("Xin chao", lines, error));
  TEST_ASSERT_EQUAL(1, lines.size());
  TEST_ASSERT_EQUAL_STRING("Xin chao", lines[0].c_str());
  TEST_ASSERT_TRUE(splitDisplayText("Dong 1\nDong 2\nDong 3\nDong 4", lines, error));
  TEST_ASSERT_EQUAL(4, lines.size());
  TEST_ASSERT_EQUAL_STRING("Dong 4", lines[3].c_str());
}

void test_display_text_gioi_han_21_ky_tu() {
  std::vector<std::string> lines;
  std::string error;
  TEST_ASSERT_TRUE(splitDisplayText("123456789012345678901", lines, error));
  TEST_ASSERT_EQUAL(21, lines[0].size());
  TEST_ASSERT_FALSE(splitDisplayText("1234567890123456789012", lines, error));
  TEST_ASSERT_TRUE(error.find("21") != std::string::npos);
}

void test_display_text_tu_choi_qua_bon_dong_va_unicode() {
  std::vector<std::string> lines;
  std::string error;
  TEST_ASSERT_FALSE(splitDisplayText("1\n2\n3\n4\n5", lines, error));
  TEST_ASSERT_TRUE(error.find("4") != std::string::npos);
  TEST_ASSERT_FALSE(splitDisplayText("Xin chào", lines, error));
  TEST_ASSERT_TRUE(error.find("ASCII") != std::string::npos);
}

void test_display_text_chap_nhan_chuoi_rong_va_chi_ascii_in_duoc() {
  std::vector<std::string> lines{"old"};
  std::string error;
  TEST_ASSERT_TRUE(splitDisplayText("", lines, error));
  TEST_ASSERT_EQUAL(0, lines.size());
  TEST_ASSERT_TRUE(splitDisplayText(" !~", lines, error));
  TEST_ASSERT_EQUAL_STRING(" !~", lines[0].c_str());
  TEST_ASSERT_FALSE(splitDisplayText("A\tB", lines, error));
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
  RUN_TEST(test_display_text_chap_nhan_mot_dong_va_bon_dong);
  RUN_TEST(test_display_text_gioi_han_21_ky_tu);
  RUN_TEST(test_display_text_tu_choi_qua_bon_dong_va_unicode);
  RUN_TEST(test_display_text_chap_nhan_chuoi_rong_va_chi_ascii_in_duoc);
  return UNITY_END();
}
