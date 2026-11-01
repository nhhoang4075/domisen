// Luật thuần của đèn: không cần phần cứng, không cần JSON.
#include <shared/core/ColorLight.h>
#include <shared/core/Light.h>
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

// ---- đèn màu WS2812 (bảng lệnh trong docs/mqtt-topics.md, mục "Đèn màu WS2812") ----
static const Rgb kRed{255, 0, 0};

static void assertColor(uint8_t r, uint8_t g, uint8_t b, const Rgb& actual) {
  TEST_ASSERT_EQUAL_UINT8(r, actual.r);
  TEST_ASSERT_EQUAL_UINT8(g, actual.g);
  TEST_ASSERT_EQUAL_UINT8(b, actual.b);
}

void test_den_mau_mac_dinh_tat_trang_am_muc_100() {
  ColorLightState s;
  TEST_ASSERT_FALSE(s.on);
  assertColor(0xFF, 0xB4, 0x6B, s.color);
  TEST_ASSERT_EQUAL(100, s.level);
}

void test_den_mau_on_true_dung_mau_va_muc_da_nho() {
  ColorLightState s = applyColorLightCommand({}, std::nullopt, kRed, 40);  // đặt đỏ, mức 40
  s = applyColorLightCommand(s, false, std::nullopt, std::nullopt);        // tắt
  s = applyColorLightCommand(s, true, std::nullopt, std::nullopt);         // bật lại
  TEST_ASSERT_TRUE(s.on);
  assertColor(255, 0, 0, s.color);
  TEST_ASSERT_EQUAL(40, s.level);
  // bật từ trạng thái mặc định thì ra trắng ấm, mức 100
  ColorLightState d = applyColorLightCommand({}, true, std::nullopt, std::nullopt);
  TEST_ASSERT_TRUE(d.on);
  assertColor(0xFF, 0xB4, 0x6B, d.color);
  TEST_ASSERT_EQUAL(100, d.level);
}

void test_den_mau_on_false_tat_nho_mau_va_muc_cu() {
  ColorLightState s = applyColorLightCommand({}, std::nullopt, kRed, 40);
  s = applyColorLightCommand(s, false, std::nullopt, std::nullopt);
  TEST_ASSERT_FALSE(s.on);
  assertColor(255, 0, 0, s.color);
  TEST_ASSERT_EQUAL(40, s.level);
}

void test_den_mau_chi_gui_color_thi_doi_mau_va_bat() {
  ColorLightState s = applyColorLightCommand({}, std::nullopt, kRed, std::nullopt);
  TEST_ASSERT_TRUE(s.on);
  assertColor(255, 0, 0, s.color);
  TEST_ASSERT_EQUAL(100, s.level);  // mức không đổi
}

void test_den_mau_chi_gui_level_thi_doi_muc_va_bat() {
  ColorLightState s = applyColorLightCommand({}, std::nullopt, std::nullopt, 40);
  TEST_ASSERT_TRUE(s.on);
  TEST_ASSERT_EQUAL(40, s.level);
  assertColor(0xFF, 0xB4, 0x6B, s.color);  // màu không đổi
}

void test_den_mau_level_0_tat_nhung_giu_muc_cu() {
  ColorLightState s = applyColorLightCommand({}, true, kRed, 60);
  s = applyColorLightCommand(s, std::nullopt, std::nullopt, 0);
  TEST_ASSERT_FALSE(s.on);
  TEST_ASSERT_EQUAL(60, s.level);
  assertColor(255, 0, 0, s.color);
}

void test_den_mau_on_ghi_ro_thang_color_va_level() {
  ColorLightState s = applyColorLightCommand({}, false, kRed, 80);  // đặt đỏ, mức 80 nhưng nói rõ tắt
  TEST_ASSERT_FALSE(s.on);
  assertColor(255, 0, 0, s.color);  // màu vẫn được lưu
  TEST_ASSERT_EQUAL(80, s.level);
  s = applyColorLightCommand(s, true, std::nullopt, 0);  // on:true thắng level 0
  TEST_ASSERT_TRUE(s.on);
  TEST_ASSERT_EQUAL(80, s.level);
}

void test_den_mau_level_ngoai_khoang_bi_cat() {
  TEST_ASSERT_EQUAL(100, applyColorLightCommand({}, std::nullopt, std::nullopt, 500).level);
  TEST_ASSERT_FALSE(applyColorLightCommand({}, std::nullopt, std::nullopt, -5).on);
}

void test_den_mau_mau_ra_thanh_led_muc_40() {
  ColorLightState s = applyColorLightCommand({}, std::nullopt, std::nullopt, 40);  // #FFB46B ở mức 40
  assertColor(102, 72, 43, colorLightOutput(s));  // 255*0,4 ; 180*0,4 ; 107*0,4 = 42,8 làm tròn 43
}

void test_den_mau_mau_ra_thanh_led_tat_va_muc_day() {
  ColorLightState s = applyColorLightCommand({}, std::nullopt, std::nullopt, 40);
  s = applyColorLightCommand(s, false, std::nullopt, std::nullopt);
  assertColor(0, 0, 0, colorLightOutput(s));  // tắt thì đen, dù đang nhớ màu
  assertColor(0xFF, 0xB4, 0x6B, colorLightOutput(applyColorLightCommand({}, true, std::nullopt, std::nullopt)));
  assertColor(3, 2, 1, colorLightOutput(applyColorLightCommand({}, true, std::nullopt, 1)));  // 2,55 ; 1,8 ; 1,07
  assertColor(128, 90, 54, colorLightOutput(applyColorLightCommand({}, true, std::nullopt, 50)));  // 127,5 ; 90 ; 53,5
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
  RUN_TEST(test_den_mau_mac_dinh_tat_trang_am_muc_100);
  RUN_TEST(test_den_mau_on_true_dung_mau_va_muc_da_nho);
  RUN_TEST(test_den_mau_on_false_tat_nho_mau_va_muc_cu);
  RUN_TEST(test_den_mau_chi_gui_color_thi_doi_mau_va_bat);
  RUN_TEST(test_den_mau_chi_gui_level_thi_doi_muc_va_bat);
  RUN_TEST(test_den_mau_level_0_tat_nhung_giu_muc_cu);
  RUN_TEST(test_den_mau_on_ghi_ro_thang_color_va_level);
  RUN_TEST(test_den_mau_level_ngoai_khoang_bi_cat);
  RUN_TEST(test_den_mau_mau_ra_thanh_led_muc_40);
  RUN_TEST(test_den_mau_mau_ra_thanh_led_tat_va_muc_day);
  return UNITY_END();
}
