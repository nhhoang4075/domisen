// Tên topic và mã hóa JSON. Giá trị mong đợi lấy từ docs/mqtt-topics.md.
#include <shared/protocol/BinarySensor.h>
#include <shared/protocol/CardReader.h>
#include <shared/protocol/Climate.h>
#include <shared/protocol/Display.h>
#include <shared/protocol/Gas.h>
#include <shared/protocol/LightSensor.h>

#include <shared/protocol/ColorLight.h>
#include <shared/protocol/Error.h>
#include <shared/protocol/Light.h>
#include <shared/protocol/Position.h>
#include <shared/protocol/Switch.h>
#include <shared/protocol/Topics.h>
#include <unity.h>

#include <vector>

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

void test_ma_hoa_cam_bien_hai_trang_thai() {
  TEST_ASSERT_EQUAL_STRING("{\"motion\":true}", encodeBinarySensorState("motion", true).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"motion\":false}", encodeBinarySensorState("motion", false).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"motion\":null}", encodeBinarySensorUnknown("motion").c_str());
}

void test_ma_hoa_dau_doc_the() {
  TEST_ASSERT_EQUAL_STRING("{\"uid\":\"A3F2197C\"}", encodeCardReaderState("A3F2197C").c_str());
  TEST_ASSERT_EQUAL_STRING("{\"uid\":null}", encodeCardReaderEmpty().c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/entrance/card-reader/state", stateTopic("entrance/card-reader").c_str());
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

void test_display_giai_ma_lenh_text() {
  std::string text, error;
  TEST_ASSERT_TRUE(decodeDisplayCommand("{\"text\":\"Xin chao\"}", text, error));
  TEST_ASSERT_EQUAL_STRING("Xin chao", text.c_str());
  TEST_ASSERT_FALSE(decodeDisplayCommand("{}", text, error));
  TEST_ASSERT_FALSE(error.empty());
  TEST_ASSERT_FALSE(decodeDisplayCommand("{\"text\":1}", text, error));
  TEST_ASSERT_FALSE(error.empty());
  TEST_ASSERT_FALSE(decodeDisplayCommand("[\"Xin chao\"]", text, error));
  TEST_ASSERT_FALSE(error.empty());
}

void test_display_ma_hoa_state() {
  TEST_ASSERT_EQUAL_STRING("{\"text\":\"\"}", encodeDisplayState("").c_str());
  TEST_ASSERT_EQUAL_STRING("{\"text\":\"Dong 1\\nDong 2\"}", encodeDisplayState("Dong 1\nDong 2").c_str());
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

// Servo cửa: danh sách vị trí theo thứ tự khai báo trong main.cpp.
void test_gas_ma_hoa_trang_thai() {
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":1830,\"alarm\":true}", encodeGasState(1830, true).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":0,\"alarm\":false}", encodeGasState(0, false).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":null,\"alarm\":false}", encodeGasWarmup().c_str());
}

void test_cam_bien_anh_sang_ma_hoa_trang_thai() {
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":2310,\"dark\":false}", encodeLightSensorState(2310, false).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":0,\"dark\":true}", encodeLightSensorState(0, true).c_str());
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":null,\"dark\":false}", encodeLightSensorUnavailable().c_str());
}

void test_giai_ma_vi_tri() {
  std::vector<std::string> names = {"closed", "open"};
  std::string pos = "closed", err;
  TEST_ASSERT_TRUE(decodePositionCommand("{\"position\":\"open\"}", names, pos, err));
  TEST_ASSERT_EQUAL_STRING("open", pos.c_str());
  TEST_ASSERT_TRUE(decodePositionCommand("{\"position\":\"closed\"}", names, pos, err));
  TEST_ASSERT_EQUAL_STRING("closed", pos.c_str());
}

void test_vi_tri_sai_bao_loi_neu_danh_sach() {
  std::vector<std::string> names = {"closed", "open"};
  std::string pos = "open", err;
  const std::string expected = "position phải là một trong: closed, open";
  TEST_ASSERT_FALSE(decodePositionCommand("{}", names, pos, err));  // thiếu trường
  TEST_ASSERT_EQUAL_STRING(expected.c_str(), err.c_str());
  TEST_ASSERT_FALSE(decodePositionCommand("{\"position\":1}", names, pos, err));  // sai kiểu
  TEST_ASSERT_EQUAL_STRING(expected.c_str(), err.c_str());
  TEST_ASSERT_FALSE(decodePositionCommand("{\"position\":\"middle\"}", names, pos, err));  // không có trong danh sách
  TEST_ASSERT_EQUAL_STRING(expected.c_str(), err.c_str());
  TEST_ASSERT_FALSE(decodePositionCommand("khong phai json", names, pos, err));
  TEST_ASSERT_FALSE(decodePositionCommand("[\"open\"]", names, pos, err));
  TEST_ASSERT_EQUAL_STRING("open", pos.c_str());  // lệnh sai không đổi vị trí
}

void test_ma_hoa_vi_tri() {
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"open\"}", encodePositionState("open").c_str());
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"closed\"}", encodePositionState("closed").c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_topics);
  RUN_TEST(test_giai_ma_lenh_day_du);
  RUN_TEST(test_giai_ma_lenh_mot_phan);
  RUN_TEST(test_lenh_sai_bi_tu_choi);
  RUN_TEST(test_ma_hoa_trang_thai);
  RUN_TEST(test_ma_hoa_khi_hau);
  RUN_TEST(test_ma_hoa_cam_bien_hai_trang_thai);
  RUN_TEST(test_ma_hoa_dau_doc_the);
  RUN_TEST(test_giai_ma_lenh_bat_tat);
  RUN_TEST(test_lenh_bat_tat_sai_bi_tu_choi);
  RUN_TEST(test_ma_hoa_trang_thai_bat_tat);
  RUN_TEST(test_ma_hoa_loi);
  RUN_TEST(test_display_giai_ma_lenh_text);
  RUN_TEST(test_display_ma_hoa_state);

  RUN_TEST(test_den_mau_giai_ma_mau_hoa_va_thuong);
  RUN_TEST(test_den_mau_giai_ma_mau_sai_dinh_dang);
  RUN_TEST(test_den_mau_ma_hoa_mau_chu_hoa);
  RUN_TEST(test_den_mau_giai_ma_lenh);
  RUN_TEST(test_den_mau_lenh_sai_bi_tu_choi_va_khong_doi_out);
  RUN_TEST(test_den_mau_ma_hoa_trang_thai);
  RUN_TEST(test_gas_ma_hoa_trang_thai);
  RUN_TEST(test_cam_bien_anh_sang_ma_hoa_trang_thai);
  RUN_TEST(test_giai_ma_vi_tri);
  RUN_TEST(test_vi_tri_sai_bao_loi_neu_danh_sach);
  RUN_TEST(test_ma_hoa_vi_tri);
  return UNITY_END();
}
