// Luật thuần của đèn, đèn màu và servo: không cần phần cứng, không cần JSON.
#include <shared/core/ColorLight.h>
#include <shared/core/Fan.h>
#include <shared/core/Light.h>
#include <shared/core/Servo.h>
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

// Giá trị mong đợi lấy từ docs/mqtt-topics.md: xung 0,5 ms đến 2,4 ms trong chu kỳ 20 ms.
void test_servo_duty_ba_goc() {
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.025f, servoDuty(0.0f));    // 0,5 / 20
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0725f, servoDuty(90.0f));  // 1,45 / 20
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.12f, servoDuty(180.0f));   // 2,4 / 20
}

void test_servo_goc_ngoai_khoang_bi_cat() {
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.025f, servoDuty(-30.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.12f, servoDuty(200.0f));
}

// Giá trị mong đợi tính tay theo docs/mqtt-topics.md: duty = min + (1 - min) * level / 100, min = 0,3.
void test_quat_duty_tat_la_0() {
  TEST_ASSERT_EQUAL_FLOAT(0.0f, fanDuty({}, 0.3f));
  LightState s = applyLightCommand({}, true, 40);
  s = applyLightCommand(s, false, std::nullopt);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, fanDuty(s, 0.3f));  // tắt thì 0, dù đang nhớ mức 40
}

void test_quat_duty_tuyen_tinh_tu_min_den_1() {
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 1.0f, fanDuty(applyLightCommand({}, true, std::nullopt), 0.3f));   // mức 100
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.65f, fanDuty(applyLightCommand({}, true, 50), 0.3f));            // 0,3 + 0,7 * 0,5
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.307f, fanDuty(applyLightCommand({}, true, 1), 0.3f));            // 0,3 + 0,7 * 0,01
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.5f, fanDuty(applyLightCommand({}, true, 50), 0.0f));             // min 0: đúng tỉ lệ
}

void test_quat_da_khoi_dong_chi_khi_dung_yen_len_duty_thap() {
  TEST_ASSERT_TRUE(fanNeedsKick(0.0f, 0.307f));   // đứng yên lên mức thấp
  TEST_ASSERT_FALSE(fanNeedsKick(0.5f, 0.307f));  // đang quay hạ xuống: không đá
  TEST_ASSERT_FALSE(fanNeedsKick(0.0f, 1.0f));    // đã hết ga rồi
  TEST_ASSERT_FALSE(fanNeedsKick(0.0f, 0.0f));    // vẫn tắt
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
  RUN_TEST(test_quat_duty_tat_la_0);
  RUN_TEST(test_quat_duty_tuyen_tinh_tu_min_den_1);
  RUN_TEST(test_quat_da_khoi_dong_chi_khi_dung_yen_len_duty_thap);
  RUN_TEST(test_servo_duty_ba_goc);
  RUN_TEST(test_servo_goc_ngoai_khoang_bi_cat);
  return UNITY_END();
}
