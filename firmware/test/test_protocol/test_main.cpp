// Tên topic và mã hóa JSON. Giá trị mong đợi lấy từ docs/mqtt-topics.md.
#include <shared/protocol/Climate.h>
#include <shared/protocol/ColorLight.h>
#include <shared/protocol/Error.h>
#include <shared/protocol/Light.h>
#include <shared/protocol/Switch.h>
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

void test_ma_hoa_khi_hau() {
  TEST_ASSERT_EQUAL_STRING("{\"temp\":27.5,\"humidity\":62}", encodeClimateState(27.5f, 62.0f).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"temp\":27.5,\"humidity\":61.3}", encodeClimateState(27.46f, 61.34f).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"temp\":-3.2,\"humidity\":100}", encodeClimateState(-3.24f, 100.0f).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"temp\":null,\"humidity\":null}", encodeClimateUnavailable().c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/living/climate/state", stateTopic("living/climate").c_str());
}

void test_giai_ma_lenh_bat_tat() {
  bool on = false; std::string err;
  TEST_ASSERT_TRUE(decodeSwitchCommand("{\"on\":true}", on, err));
  TEST_ASSERT_TRUE(on);
  TEST_ASSERT_TRUE(decodeSwitchCommand("{\"on\":false}", on, err));
  TEST_ASSERT_FALSE(on);
}

void test_lenh_bat_tat_sai_bi_tu_choi() {
  bool on = true; std::string err;
  TEST_ASSERT_FALSE(decodeSwitchCommand("khong phai json", on, err));
  TEST_ASSERT_FALSE(decodeSwitchCommand("[true]", on, err));
  TEST_ASSERT_FALSE(decodeSwitchCommand("{}", on, err));
  TEST_ASSERT_FALSE(decodeSwitchCommand("{\"level\":50}", on, err));
  TEST_ASSERT_FALSE(decodeSwitchCommand("{\"on\":\"yes\"}", on, err));
  TEST_ASSERT_FALSE(decodeSwitchCommand("{\"on\":1}", on, err));
  TEST_ASSERT_TRUE(on);  // lệnh sai không được đổi giá trị
  TEST_ASSERT_FALSE(err.empty());
}

void test_ma_hoa_trang_thai_bat_tat() {
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", encodeSwitchState(true).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", encodeSwitchState(false).c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/living/fan/cmd", cmdTopic("living/fan").c_str());
}

void test_ma_hoa_loi() {
  TEST_ASSERT_EQUAL_STRING("{\"error\":\"hỏng\"}", encodeError("hỏng").c_str());
}

// ---- đèn màu WS2812 ----
void test_den_mau_giai_ma_mau_hoa_va_thuong() {
  domisen::core::Rgb c;
  TEST_ASSERT_TRUE(parseHexColor("#FFB46B", c));
  TEST_ASSERT_EQUAL_UINT8(255, c.r);
  TEST_ASSERT_EQUAL_UINT8(180, c.g);
  TEST_ASSERT_EQUAL_UINT8(107, c.b);
  domisen::core::Rgb lower;
  TEST_ASSERT_TRUE(parseHexColor("#ffb46b", lower));
  TEST_ASSERT_TRUE(c == lower);
  TEST_ASSERT_TRUE(parseHexColor("#00aAfF", c));
  TEST_ASSERT_EQUAL_UINT8(0, c.r);
  TEST_ASSERT_EQUAL_UINT8(170, c.g);
  TEST_ASSERT_EQUAL_UINT8(255, c.b);
}

void test_den_mau_giai_ma_mau_sai_dinh_dang() {
  domisen::core::Rgb c;
  TEST_ASSERT_FALSE(parseHexColor("", c));
  TEST_ASSERT_FALSE(parseHexColor("FF0000", c));     // thiếu #
  TEST_ASSERT_FALSE(parseHexColor("#FFF", c));       // dạng ngắn không nhận
  TEST_ASSERT_FALSE(parseHexColor("#GGGGGG", c));    // không phải hex
  TEST_ASSERT_FALSE(parseHexColor("#FF00000", c));   // thừa ký tự
  TEST_ASSERT_FALSE(parseHexColor("#FF 000", c));
  TEST_ASSERT_FALSE(parseHexColor("đỏ", c));
}

void test_den_mau_ma_hoa_mau_chu_hoa() {
  TEST_ASSERT_EQUAL_STRING("#FF0000", formatHexColor({255, 0, 0}).c_str());
  TEST_ASSERT_EQUAL_STRING("#FFB46B", formatHexColor({255, 180, 107}).c_str());
  TEST_ASSERT_EQUAL_STRING("#000A0F", formatHexColor({0, 10, 15}).c_str());
}

void test_den_mau_giai_ma_lenh() {
  ColorLightCommand c; std::string err;
  TEST_ASSERT_TRUE(decodeColorLightCommand("{\"on\":true,\"color\":\"#ff0000\",\"level\":40}", c, err));
  TEST_ASSERT_TRUE(*c.on);
  TEST_ASSERT_EQUAL_UINT8(255, c.color->r);
  TEST_ASSERT_EQUAL_UINT8(0, c.color->g);
  TEST_ASSERT_EQUAL(40, *c.level);
  // mỗi trường tùy chọn
  TEST_ASSERT_TRUE(decodeColorLightCommand("{\"on\":false}", c, err));
  TEST_ASSERT_FALSE(*c.on);
  TEST_ASSERT_FALSE(c.color.has_value());
  TEST_ASSERT_FALSE(c.level.has_value());
  TEST_ASSERT_TRUE(decodeColorLightCommand("{\"color\":\"#FF0000\"}", c, err));
  TEST_ASSERT_FALSE(c.on.has_value());
  TEST_ASSERT_TRUE(c.color.has_value());
  TEST_ASSERT_TRUE(decodeColorLightCommand("{\"level\":40}", c, err));
  TEST_ASSERT_FALSE(c.color.has_value());
  TEST_ASSERT_EQUAL(40, *c.level);
}

void test_den_mau_lenh_sai_bi_tu_choi_va_khong_doi_out() {
  ColorLightCommand c; std::string err;
  TEST_ASSERT_TRUE(decodeColorLightCommand("{\"level\":40}", c, err));  // đặt giá trị cũ để thử "không đổi"
  TEST_ASSERT_FALSE(decodeColorLightCommand("khong phai json", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("[1,2]", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{}", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"on\":\"yes\"}", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"level\":\"cao\"}", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"color\":\"đỏ\"}", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"color\":\"#GGG\"}", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"color\":\"#FFF\"}", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"color\":16711680}", c, err));  // sai kiểu: số
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"color\":true}", c, err));
  TEST_ASSERT_FALSE(decodeColorLightCommand("{\"on\":true,\"color\":\"#GGG\"}", c, err));
  TEST_ASSERT_FALSE(err.empty());
  // lệnh sai không được ghi vào out
  TEST_ASSERT_FALSE(c.on.has_value());
  TEST_ASSERT_FALSE(c.color.has_value());
  TEST_ASSERT_EQUAL(40, *c.level);
}

void test_den_mau_ma_hoa_trang_thai() {
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"color\":\"#FF0000\",\"level\":40}",
                           encodeColorLightState(true, {255, 0, 0}, 40).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"color\":\"#FFB46B\",\"level\":100}",
                           encodeColorLightState(false, {255, 180, 107}, 100).c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_topics);
  RUN_TEST(test_giai_ma_lenh_day_du);
  RUN_TEST(test_giai_ma_lenh_mot_phan);
  RUN_TEST(test_lenh_sai_bi_tu_choi);
  RUN_TEST(test_ma_hoa_trang_thai);
  RUN_TEST(test_ma_hoa_khi_hau);
  RUN_TEST(test_giai_ma_lenh_bat_tat);
  RUN_TEST(test_lenh_bat_tat_sai_bi_tu_choi);
  RUN_TEST(test_ma_hoa_trang_thai_bat_tat);
  RUN_TEST(test_ma_hoa_loi);
  RUN_TEST(test_den_mau_giai_ma_mau_hoa_va_thuong);
  RUN_TEST(test_den_mau_giai_ma_mau_sai_dinh_dang);
  RUN_TEST(test_den_mau_ma_hoa_mau_chu_hoa);
  RUN_TEST(test_den_mau_giai_ma_lenh);
  RUN_TEST(test_den_mau_lenh_sai_bi_tu_choi_va_khong_doi_out);
  RUN_TEST(test_den_mau_ma_hoa_trang_thai);
  return UNITY_END();
}
