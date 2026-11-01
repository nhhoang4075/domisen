// Hub và DimmableLight, chạy với bản giả: không cần board, không cần mạng.
#include <shared/adapters_fake/FakeClimateSensor.h>
#include <shared/adapters_fake/FakeClock.h>
#include <shared/adapters_fake/FakePwm.h>
#include <shared/adapters_fake/FakeTransport.h>
#include <shared/services/ClimateDevice.h>
#include <shared/services/DimmableLight.h>
#include <shared/services/Hub.h>
#include <unity.h>

using namespace domisen;
using adapters::fake::FakeClimateSensor;
using adapters::fake::FakeClock;
using adapters::fake::FakePwm;
using adapters::fake::FakeTransport;
using services::ClimateDevice;
using services::DimmableLight;
using services::Hub;

static const char* CEILING_CMD = "domisen/living/ceiling-light/cmd";
static const char* CEILING_STATE = "domisen/living/ceiling-light/state";

// Mỗi test dựng một board mới: hai đèn dùng chung một lớp, khác id và PWM.
struct Rig {
  FakeTransport transport;
  FakeClock clock;
  FakePwm ceilingPwm, nightPwm;
  DimmableLight ceiling{"living/ceiling-light", ceilingPwm};
  DimmableLight night{"bedroom/night-light", nightPwm};
  Hub hub{"comfort", transport, clock, 5000};

  Rig() {
    hub.add(ceiling);
    hub.add(night);
    hub.begin();
  }
};

void setUp() {}
void tearDown() {}

void test_begin_dang_ky_topic_lenh_va_ping() {
  Rig r;
  TEST_ASSERT_EQUAL(3, r.transport.subscriptions.size());
  TEST_ASSERT_EQUAL_STRING("domisen/node/comfort/ping", r.transport.subscriptions[0].c_str());
  TEST_ASSERT_EQUAL_STRING(CEILING_CMD, r.transport.subscriptions[1].c_str());
  TEST_ASSERT_EQUAL_STRING("domisen/bedroom/night-light/cmd", r.transport.subscriptions[2].c_str());
}

void test_den_tat_luc_khoi_dong() {
  Rig r;
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.ceilingPwm.last);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.nightPwm.last);
}

void test_noi_broker_bao_online_va_trang_thai_retain() {
  Rig r;
  r.transport.simulateConnect();
  const auto* status = r.transport.last("domisen/node/comfort/status");
  TEST_ASSERT_NOT_NULL(status);
  TEST_ASSERT_EQUAL_STRING("online", status->payload.c_str());
  TEST_ASSERT_TRUE(status->retain);
  const auto* state = r.transport.last(CEILING_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":100}", state->payload.c_str());
  TEST_ASSERT_TRUE(state->retain);
}

void test_lenh_dieu_khien_pwm_va_bao_lai_trang_thai() {
  Rig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":true,\"level\":50}");
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2176f, r.ceilingPwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"level\":50}", r.transport.last(CEILING_STATE)->payload.c_str());
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":false}");
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.ceilingPwm.last);
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":true}");  // bật lại đúng mức 50
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2176f, r.ceilingPwm.last);
}

void test_hai_den_doc_lap() {
  Rig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage("domisen/bedroom/night-light/cmd", "{\"on\":true,\"level\":100}");
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.nightPwm.last);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.ceilingPwm.last);
}

void test_lenh_sai_bao_loi_va_khong_doi_trang_thai() {
  Rig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":true,\"level\":50}");
  int stateCount = r.transport.count(CEILING_STATE);
  r.transport.simulateMessage(CEILING_CMD, "rac");
  const auto* err = r.transport.last("domisen/living/ceiling-light/error");
  TEST_ASSERT_NOT_NULL(err);
  TEST_ASSERT_FALSE(err->retain);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.2176f, r.ceilingPwm.last);  // vẫn như cũ
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(CEILING_STATE));
}

void test_noi_lai_gui_lai_trang_thai_hien_tai() {
  Rig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":true,\"level\":30}");
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"level\":30}", r.transport.last(CEILING_STATE)->payload.c_str());
}

void test_ping_tra_pong() {
  Rig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage("domisen/node/comfort/ping", "xin chao");
  const auto* pong = r.transport.last("domisen/node/comfort/pong");
  TEST_ASSERT_NOT_NULL(pong);
  TEST_ASSERT_EQUAL_STRING("xin chao", pong->payload.c_str());
}

void test_topic_la_bi_bo_qua() {
  Rig r;
  r.transport.simulateConnect();
  size_t before = r.transport.published.size();
  r.transport.simulateMessage("domisen/kitchen/gas/cmd", "{\"on\":true}");  // chưa đăng ký nên không tới
  TEST_ASSERT_EQUAL(before, r.transport.published.size());
}

void test_nhip_uptime() {
  Rig r;
  r.transport.simulateConnect();
  r.hub.loop();
  TEST_ASSERT_EQUAL(0, r.transport.count("domisen/node/comfort/uptime"));  // chưa tới 5 giây
  r.clock.advance(5000);
  r.hub.loop();
  TEST_ASSERT_EQUAL_STRING("5", r.transport.last("domisen/node/comfort/uptime")->payload.c_str());
  r.clock.advance(2000);
  r.hub.loop();
  TEST_ASSERT_EQUAL(1, r.transport.count("domisen/node/comfort/uptime"));
  r.clock.advance(3000);
  r.hub.loop();
  TEST_ASSERT_EQUAL_STRING("10", r.transport.last("domisen/node/comfort/uptime")->payload.c_str());
}

// millis() quay về 0 sau khoảng 49,7 ngày, nhưng uptime gửi lên không được bị đặt lại.
void test_uptime_khong_bi_dat_lai_khi_dong_ho_quay_vong() {
  Rig r;
  r.transport.simulateConnect();
  r.clock.set(4294965295u);  // 2000 ms nữa là quay vòng
  r.hub.loop();
  TEST_ASSERT_EQUAL_STRING("4294965", r.transport.last("domisen/node/comfort/uptime")->payload.c_str());
  r.clock.advance(10000);    // đồng hồ đã quay về giá trị nhỏ
  r.hub.loop();
  TEST_ASSERT_EQUAL_STRING("4294975", r.transport.last("domisen/node/comfort/uptime")->payload.c_str());
}

void test_mat_ket_noi_thi_khong_gui_uptime() {
  Rig r;
  r.transport.simulateConnect();
  r.transport.simulateDisconnect();
  r.clock.advance(6000);
  r.hub.loop();
  TEST_ASSERT_EQUAL(0, r.transport.count("domisen/node/comfort/uptime"));
}

// ---- cảm biến nhiệt độ, độ ẩm ----
static const char* CLIMATE_STATE = "domisen/living/climate/state";

struct ClimateRig {
  FakeTransport transport;
  FakeClock clock;
  FakeClimateSensor sensor;
  ClimateDevice climate{"living/climate", sensor, 10000};
  Hub hub{"comfort", transport, clock, 5000};

  ClimateRig() {
    sensor.next = core::ClimateReading{27.5f, 62.0f};
    hub.add(climate);
    hub.begin();
  }
};

void test_khi_hau_do_ngay_khi_noi_va_gui_state_retain() {
  ClimateRig r;
  r.transport.simulateConnect();
  r.hub.loop();  // lần tick đầu đo ngay, không đợi hết chu kỳ
  TEST_ASSERT_EQUAL(1, r.sensor.reads);
  const auto* state = r.transport.last(CLIMATE_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"temp\":27.5,\"humidity\":62}", state->payload.c_str());
  TEST_ASSERT_TRUE(state->retain);
}

void test_khi_hau_do_moi_10_giay() {
  ClimateRig r;
  r.transport.simulateConnect();
  r.hub.loop();
  int before = r.transport.count(CLIMATE_STATE);
  r.clock.advance(9999);
  r.hub.loop();
  TEST_ASSERT_EQUAL(before, r.transport.count(CLIMATE_STATE));
  TEST_ASSERT_EQUAL(1, r.sensor.reads);
  r.sensor.next = core::ClimateReading{28.1f, 60.0f};
  r.clock.advance(1);
  r.hub.loop();
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(CLIMATE_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"temp\":28.1,\"humidity\":60}", r.transport.last(CLIMATE_STATE)->payload.c_str());
}

void test_khi_hau_doc_loi_gui_null() {
  ClimateRig r;
  r.transport.simulateConnect();
  r.hub.loop();
  r.sensor.next = std::nullopt;
  r.clock.advance(10000);
  r.hub.loop();
  TEST_ASSERT_EQUAL_STRING("{\"temp\":null,\"humidity\":null}", r.transport.last(CLIMATE_STATE)->payload.c_str());
}

void test_khi_hau_mat_ket_noi_thi_khong_do() {
  ClimateRig r;
  r.transport.simulateConnect();
  r.transport.simulateDisconnect();
  r.clock.advance(30000);
  r.hub.loop();
  TEST_ASSERT_EQUAL(0, r.sensor.reads);
}

void test_khi_hau_chua_do_thi_state_la_null() {
  ClimateRig r;
  r.transport.simulateConnect();  // chưa tick nên chưa đo được lần nào
  TEST_ASSERT_EQUAL_STRING("{\"temp\":null,\"humidity\":null}", r.transport.last(CLIMATE_STATE)->payload.c_str());
}

void test_khi_hau_noi_lai_gui_lai_so_do_gan_nhat() {
  ClimateRig r;
  r.transport.simulateConnect();
  r.hub.loop();
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL(3, r.transport.count(CLIMATE_STATE));  // null lúc nối, số đo, rồi gửi lại số đo
  TEST_ASSERT_EQUAL_STRING("{\"temp\":27.5,\"humidity\":62}", r.transport.last(CLIMATE_STATE)->payload.c_str());
}

void test_khi_hau_chi_doc_khong_nhan_lenh() {
  ClimateRig r;
  r.transport.simulateConnect();
  r.hub.loop();
  int stateCount = r.transport.count(CLIMATE_STATE);
  r.transport.simulateMessage("domisen/living/climate/cmd", "{\"on\":true}");
  TEST_ASSERT_NOT_NULL(r.transport.last("domisen/living/climate/error"));
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(CLIMATE_STATE));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_begin_dang_ky_topic_lenh_va_ping);
  RUN_TEST(test_den_tat_luc_khoi_dong);
  RUN_TEST(test_noi_broker_bao_online_va_trang_thai_retain);
  RUN_TEST(test_lenh_dieu_khien_pwm_va_bao_lai_trang_thai);
  RUN_TEST(test_hai_den_doc_lap);
  RUN_TEST(test_lenh_sai_bao_loi_va_khong_doi_trang_thai);
  RUN_TEST(test_noi_lai_gui_lai_trang_thai_hien_tai);
  RUN_TEST(test_ping_tra_pong);
  RUN_TEST(test_topic_la_bi_bo_qua);
  RUN_TEST(test_nhip_uptime);
  RUN_TEST(test_uptime_khong_bi_dat_lai_khi_dong_ho_quay_vong);
  RUN_TEST(test_mat_ket_noi_thi_khong_gui_uptime);
  RUN_TEST(test_khi_hau_do_ngay_khi_noi_va_gui_state_retain);
  RUN_TEST(test_khi_hau_do_moi_10_giay);
  RUN_TEST(test_khi_hau_doc_loi_gui_null);
  RUN_TEST(test_khi_hau_mat_ket_noi_thi_khong_do);
  RUN_TEST(test_khi_hau_chua_do_thi_state_la_null);
  RUN_TEST(test_khi_hau_noi_lai_gui_lai_so_do_gan_nhat);
  RUN_TEST(test_khi_hau_chi_doc_khong_nhan_lenh);
  return UNITY_END();
}
