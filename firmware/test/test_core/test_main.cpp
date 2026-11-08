// Luật thuần của đèn, đèn màu và servo: không cần phần cứng, không cần JSON.
#include <shared/core/ColorLight.h>
#include <shared/core/Fan.h>
#include <shared/core/Gas.h>
#include <shared/core/Light.h>
#include <shared/core/DisplayText.h>

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

void test_display_text_gioi_han_16_ky_tu() {
  std::vector<std::string> lines;
  std::string error;
  TEST_ASSERT_TRUE(splitDisplayText("1234567890123456", lines, error));
  TEST_ASSERT_EQUAL(16, lines[0].size());
  TEST_ASSERT_FALSE(splitDisplayText("12345678901234567", lines, error));
  TEST_ASSERT_TRUE(error.find("16") != std::string::npos);
  TEST_ASSERT_EQUAL(0, lines.size());
}

void test_display_text_tu_choi_qua_bon_dong() {
  std::vector<std::string> lines;
  std::string error;
  TEST_ASSERT_FALSE(splitDisplayText("1\n2\n3\n4\n5", lines, error));
  TEST_ASSERT_TRUE(error.find("4") != std::string::npos);
}

void test_display_text_chap_nhan_chuoi_rong_va_chi_ascii_in_duoc() {
  std::vector<std::string> lines{"old"};
  std::string error;
  TEST_ASSERT_TRUE(splitDisplayText("", lines, error));
  TEST_ASSERT_EQUAL(0, lines.size());
  TEST_ASSERT_TRUE(splitDisplayText(" !~", lines, error));
  TEST_ASSERT_EQUAL_STRING(" !~", lines[0].c_str());
  TEST_ASSERT_FALSE(splitDisplayText("A\tB", lines, error));
  TEST_ASSERT_FALSE(splitDisplayText("A\x7f", lines, error));
}

// Đếm theo ký tự, không theo byte: 16 chữ có dấu (3 byte mỗi chữ) vẫn vừa một dòng, 17 chữ thì không.
void test_display_text_chap_nhan_tieng_viet_co_dau_va_dem_theo_ky_tu() {
  std::vector<std::string> lines;
  std::string error;
  TEST_ASSERT_TRUE(splitDisplayText("Xin chào", lines, error));
  TEST_ASSERT_EQUAL_STRING("Xin chào", lines[0].c_str());
  TEST_ASSERT_TRUE(splitDisplayText("Đèn phòng khách\nBật", lines, error));
  TEST_ASSERT_EQUAL(2, lines.size());
  TEST_ASSERT_TRUE(splitDisplayText("ẠẠẠẠẠẠẠẠẠẠẠẠẠẠẠẠ", lines, error));
  TEST_ASSERT_EQUAL_STRING("ẠẠẠẠẠẠẠẠẠẠẠẠẠẠẠẠ", lines[0].c_str());
  TEST_ASSERT_FALSE(splitDisplayText("ẠẠẠẠẠẠẠẠẠẠẠẠẠẠẠẠẠ", lines, error));
  TEST_ASSERT_TRUE(error.find("16") != std::string::npos);
}

// Đủ 146 chữ cái tiếng Việt (12 chữ gốc x 2 kiểu chữ, đủ năm dấu thanh, và đ Đ): mỗi dòng dưới đây có 16 chữ hoặc ít hơn.
void test_display_text_chap_nhan_du_chu_cai_tieng_viet() {
  static const char* const kLetterRows[] = {
    "aàáảãạăằắẳẵặâầấẩ",
    "ẫậeèéẻẽẹêềếểễệiì",
    "íỉĩịoòóỏõọôồốổỗộ",
    "ơờớởỡợuùúủũụưừứử",
    "ữựyỳýỷỹỵAÀÁẢÃẠĂẰ",
    "ẮẲẴẶÂẦẤẨẪẬEÈÉẺẼẸ",
    "ÊỀẾỂỄỆIÌÍỈĨỊOÒÓỎ",
    "ÕỌÔỒỐỔỖỘƠỜỚỞỠỢUÙ",
    "ÚỦŨỤƯỪỨỬỮỰYỲÝỶỸỴ",
    "đĐ",
  };
  for (const char* row : kLetterRows) {
    std::vector<std::string> lines;
    std::string error;
    TEST_ASSERT_TRUE_MESSAGE(splitDisplayText(row, lines, error), row);
    TEST_ASSERT_EQUAL(1, lines.size());
  }
}

void test_display_text_tu_choi_ky_tu_ngoai_tap_cho_phep_va_utf8_sai() {
  std::vector<std::string> lines;
  std::string error;
  TEST_ASSERT_FALSE(splitDisplayText("a\xCC\x81", lines, error));      // chữ a cộng dấu sắc rời (tổ hợp)
  TEST_ASSERT_TRUE(error.find("ASCII") != std::string::npos);
  TEST_ASSERT_FALSE(splitDisplayText("ñ", lines, error));
  TEST_ASSERT_FALSE(splitDisplayText("€", lines, error));
  TEST_ASSERT_FALSE(splitDisplayText("\xF0\x9F\x98\x80", lines, error));  // biểu tượng cảm xúc 4 byte
  TEST_ASSERT_FALSE(splitDisplayText("\xC3", lines, error));          // thiếu byte
  TEST_ASSERT_FALSE(splitDisplayText("\xC0\xA0", lines, error));      // mã quá dài
  TEST_ASSERT_FALSE(splitDisplayText("\xE0\x80\xA0", lines, error)); // mã quá dài 3 byte
  TEST_ASSERT_FALSE(splitDisplayText("\xFF", lines, error));
}

// ---- báo động gas có trễ (docs/mqtt-topics.md, mục "Cảm biến gas MQ-2") ----
void test_gas_alarm_bat_khi_dat_nguong_bat() {
  TEST_ASSERT_FALSE(gasAlarm(false, 1999, 2000, 1800));
  TEST_ASSERT_TRUE(gasAlarm(false, 2000, 2000, 1800));
  TEST_ASSERT_TRUE(gasAlarm(false, 4095, 2000, 1800));
}

void test_gas_alarm_chi_tat_khi_xuong_duoi_nguong_tat() {
  TEST_ASSERT_TRUE(gasAlarm(true, 1800, 2000, 1800));
  TEST_ASSERT_FALSE(gasAlarm(true, 1799, 2000, 1800));
  TEST_ASSERT_FALSE(gasAlarm(true, 0, 2000, 1800));
}

void test_gas_alarm_giu_nguyen_o_giua_hai_nguong() {
  TEST_ASSERT_FALSE(gasAlarm(false, 1900, 2000, 1800));  // chưa báo động thì không tự bật ở giữa
  TEST_ASSERT_TRUE(gasAlarm(true, 1900, 2000, 1800));    // đang báo động thì không tự tắt ở giữa
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

void test_quat_kick_start_chi_khi_dung_yen_len_duty_thap() {
  TEST_ASSERT_TRUE(fanNeedsKick(0.0f, 0.307f));   // đứng yên lên mức thấp
  TEST_ASSERT_FALSE(fanNeedsKick(0.5f, 0.307f));  // đang quay hạ xuống: không kick
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
  RUN_TEST(test_display_text_chap_nhan_mot_dong_va_bon_dong);
  RUN_TEST(test_gas_alarm_bat_khi_dat_nguong_bat);
  RUN_TEST(test_gas_alarm_chi_tat_khi_xuong_duoi_nguong_tat);
  RUN_TEST(test_gas_alarm_giu_nguyen_o_giua_hai_nguong);
  RUN_TEST(test_display_text_gioi_han_16_ky_tu);
  RUN_TEST(test_display_text_tu_choi_qua_bon_dong);
  RUN_TEST(test_display_text_chap_nhan_chuoi_rong_va_chi_ascii_in_duoc);
  RUN_TEST(test_display_text_chap_nhan_tieng_viet_co_dau_va_dem_theo_ky_tu);
  RUN_TEST(test_display_text_chap_nhan_du_chu_cai_tieng_viet);
  RUN_TEST(test_display_text_tu_choi_ky_tu_ngoai_tap_cho_phep_va_utf8_sai);

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
  RUN_TEST(test_quat_kick_start_chi_khi_dung_yen_len_duty_thap);
  RUN_TEST(test_servo_duty_ba_goc);
  RUN_TEST(test_servo_goc_ngoai_khoang_bi_cat);
  return UNITY_END();
}
