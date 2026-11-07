// Hub, đèn, cảm biến, quạt, servo và đầu đọc thẻ, chạy với bản giả: không cần board, không cần mạng.
#include <shared/adapters_fake/FakeCardReader.h>
#include <shared/adapters_fake/FakeClimateSensor.h>
#include <shared/adapters_fake/FakeClock.h>
#include <shared/adapters_fake/FakeDigitalInput.h>
#include <shared/adapters_fake/FakeDigitalOutput.h>
#include <shared/adapters_fake/FakePixelStrip.h>
#include <shared/adapters_fake/FakePwm.h>
#include <shared/adapters_fake/FakeTransport.h>
#include <shared/services/CardReaderDevice.h>
#include <shared/services/ClimateDevice.h>
#include <shared/services/BinarySensorDevice.h>
#include <shared/services/ColorLight.h>
#include <shared/services/DimmableLight.h>
#include <shared/services/Hub.h>
#include <shared/services/PositionDevice.h>
#include <shared/services/SpeedFan.h>
#include <shared/services/SwitchDevice.h>
#include <unity.h>

using namespace domisen;
using adapters::fake::FakeCardReader;
using adapters::fake::FakeClimateSensor;
using adapters::fake::FakeClock;
using adapters::fake::FakeDigitalInput;
using adapters::fake::FakeDigitalOutput;
using adapters::fake::FakePixelStrip;
using adapters::fake::FakePwm;
using adapters::fake::FakeTransport;
using services::CardReaderDevice;
using services::ClimateDevice;
using services::BinarySensorDevice;
using services::ColorLight;
using services::DimmableLight;
using services::Hub;
using services::PositionDevice;
using services::SpeedFan;
using services::SwitchDevice;

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

// ---- quạt phòng khách chỉnh tốc độ (SpeedFan: duty tối thiểu 0,3, kick-start 300 ms) ----
static const char* LIVING_FAN_CMD = "domisen/living/fan/cmd";
static const char* LIVING_FAN_STATE = "domisen/living/fan/state";
static const uint32_t kKickMs = 300;

struct SpeedFanRig {
  FakeTransport transport;
  FakeClock clock;
  FakePwm pwm;
  SpeedFan fan{"living/fan", pwm, clock, 0.3f, kKickMs};
  Hub hub{"comfort", transport, clock, 5000};

  SpeedFanRig() {
    pwm.last = 0.9f;  // giả sử đầu ra đang chạy, begin() phải đưa về tắt
    hub.add(fan);
    hub.begin();
  }
  // Cho hết thời gian kick-start rồi để Hub gọi tick.
  void settle() {
    clock.advance(kKickMs);
    hub.loop();
  }
};

void test_quat_toc_do_tat_luc_khoi_dong() {
  SpeedFanRig r;
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
  r.transport.simulateConnect();
  const auto* state = r.transport.last(LIVING_FAN_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":100}", state->payload.c_str());
  TEST_ASSERT_TRUE(state->retain);
}

void test_quat_toc_do_lenh_cu_on_van_chay() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"on\":true}");
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, r.pwm.last);  // mặc định mức 100
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"on\":false}");
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
}

void test_quat_toc_do_level_ra_duty_tuyen_tinh_tu_min() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":50}");  // đặt mức và tự bật
  r.settle();
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.65f, r.pwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"level\":50}", r.transport.last(LIVING_FAN_STATE)->payload.c_str());
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":1}");  // đang quay: hạ ngay, không kick
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.307f, r.pwm.last);
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":0}");  // tắt, giữ mức cũ
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
}

void test_quat_toc_do_dung_yen_len_muc_thap_thi_kick_start() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":1}");
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);  // kick: hết ga
  r.clock.advance(kKickMs - 1);
  r.hub.loop();
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);  // chưa đủ thời gian
  r.clock.advance(1);
  r.hub.loop();
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.307f, r.pwm.last);  // hạ về mức đã đặt
  r.hub.loop();
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.307f, r.pwm.last);  // và giữ nguyên
}

void test_quat_toc_do_bat_lai_sau_khi_tat_thi_lai_kick() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":10}");
  r.settle();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"on\":false}");
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"on\":true}");  // đứng yên rồi bật lại đúng mức 10
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
  r.settle();
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.37f, r.pwm.last);  // 0,3 + 0,7 * 0,1
}

void test_quat_toc_do_doi_muc_trong_luc_da_thi_doi_muc_dich() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":1}");
  r.clock.advance(100);
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":50}");
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);  // vẫn đang kick
  r.clock.advance(kKickMs - 100);             // hết kick tính từ lúc lệnh đầu, không kéo dài thêm
  r.hub.loop();
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.65f, r.pwm.last);
}

void test_quat_toc_do_tat_trong_luc_da_thi_dung_han_va_khong_chay_lai() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":1}");
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"on\":false}");
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
  r.settle();
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);  // hết thời gian kick cũng không tự bật lại
}

void test_quat_toc_do_len_het_ga_thi_khong_can_kick() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":100}");
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
  r.settle();
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
}

void test_quat_toc_do_lenh_sai_bao_loi_va_giu_nguyen() {
  SpeedFanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":50}");
  r.settle();
  int stateCount = r.transport.count(LIVING_FAN_STATE);
  r.transport.simulateMessage(LIVING_FAN_CMD, "{\"level\":\"nhanh\"}");
  TEST_ASSERT_NOT_NULL(r.transport.last("domisen/living/fan/error"));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.65f, r.pwm.last);  // vẫn chạy như cũ
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(LIVING_FAN_STATE));
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

// ---- quạt bật tắt ----
static const char* FAN_CMD = "domisen/living/fan/cmd";
static const char* FAN_STATE = "domisen/living/fan/state";

struct FanRig {
  FakeTransport transport;
  FakeClock clock;
  FakeDigitalOutput relay;
  SwitchDevice fan{"living/fan", relay};
  Hub hub{"comfort", transport, clock, 5000};

  FanRig() {
    relay.last = true;  // giả sử đầu ra đang bật, begin() phải đưa về tắt
    hub.add(fan);
    hub.begin();
  }
};

void test_quat_tat_luc_khoi_dong() {
  FanRig r;
  TEST_ASSERT_FALSE(r.relay.last);
  r.transport.simulateConnect();
  const auto* state = r.transport.last(FAN_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", state->payload.c_str());
  TEST_ASSERT_TRUE(state->retain);
}

void test_quat_bat_tat_va_bao_lai_state() {
  FanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(FAN_CMD, "{\"on\":true}");
  TEST_ASSERT_TRUE(r.relay.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(FAN_STATE)->payload.c_str());
  r.transport.simulateMessage(FAN_CMD, "{\"on\":false}");
  TEST_ASSERT_FALSE(r.relay.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(FAN_STATE)->payload.c_str());
}

void test_quat_lenh_sai_bao_loi_va_giu_nguyen() {
  FanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(FAN_CMD, "{\"on\":true}");
  int stateCount = r.transport.count(FAN_STATE);
  r.transport.simulateMessage(FAN_CMD, "{\"level\":50}");
  const auto* err = r.transport.last("domisen/living/fan/error");
  TEST_ASSERT_NOT_NULL(err);
  TEST_ASSERT_FALSE(err->retain);
  TEST_ASSERT_TRUE(r.relay.last);  // vẫn bật như cũ
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(FAN_STATE));
}

void test_quat_noi_lai_gui_lai_state() {
  FanRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(FAN_CMD, "{\"on\":true}");
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(FAN_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.relay.last);  // mất mạng thì quạt giữ nguyên trạng thái
}

// ---- cảm biến nhị phân ----
static const char* MOTION_CMD = "domisen/living/motion/cmd";
static const char* MOTION_STATE = "domisen/living/motion/state";

struct BinarySensorRig {
  FakeTransport transport;
  FakeClock clock;
  FakeDigitalInput input;
  BinarySensorDevice motion{"living/motion", "motion", input, false};
  Hub hub{"comfort", transport, clock, 5000};

  BinarySensorRig() {
    hub.add(motion);
    hub.begin();
  }
};

void test_cam_bien_nhi_phan_chua_doc_gui_null_va_lan_dau_nhan_ngay() {
  BinarySensorRig r;
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"motion\":null}", r.transport.last(MOTION_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(MOTION_STATE)->retain);

  r.hub.loop();  // lần đọc đầu được nhận ngay
  TEST_ASSERT_EQUAL_STRING("{\"motion\":false}", r.transport.last(MOTION_STATE)->payload.c_str());
  TEST_ASSERT_EQUAL(2, r.transport.count(MOTION_STATE));
}

void test_cam_bien_nhi_phan_chong_nhieu_va_nhan_gia_tri_on_dinh_sau_50ms() {
  BinarySensorRig r;
  r.transport.simulateConnect();
  r.hub.loop();  // baseline false
  int before = r.transport.count(MOTION_STATE);

  r.input.value = true;
  r.hub.loop();  // bắt đầu khoảng ổn định
  r.clock.advance(49);
  r.hub.loop();
  TEST_ASSERT_EQUAL(before, r.transport.count(MOTION_STATE));

  r.clock.advance(1);
  r.hub.loop();
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(MOTION_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"motion\":true}", r.transport.last(MOTION_STATE)->payload.c_str());
}

void test_cam_bien_nhi_phan_bo_qua_nhieu_ngan_hon_50ms() {
  BinarySensorRig r;
  r.transport.simulateConnect();
  r.hub.loop();  // baseline false
  int before = r.transport.count(MOTION_STATE);

  r.input.value = true;
  r.hub.loop();
  r.clock.advance(20);
  r.input.value = false;
  r.hub.loop();
  r.clock.advance(50);
  r.hub.loop();
  TEST_ASSERT_EQUAL(before, r.transport.count(MOTION_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"motion\":false}", r.transport.last(MOTION_STATE)->payload.c_str());
}

void test_cam_bien_nhi_phan_active_low_dao_muc() {
  FakeDigitalInput input;
  input.value = false;  // LOW nghĩa là có mưa theo tài liệu
  BinarySensorDevice rain("balcony/rain", "raining", input, true);
  TEST_ASSERT_TRUE(rain.tick(0));
  TEST_ASSERT_EQUAL_STRING("{\"raining\":true}", rain.stateJson().c_str());
}

void test_cam_bien_nhi_phan_chi_doc_va_khong_doi_state_khi_nhan_lenh() {
  BinarySensorRig r;
  r.transport.simulateConnect();
  r.hub.loop();
  int before = r.transport.count(MOTION_STATE);

  r.transport.simulateMessage(MOTION_CMD, "{\"on\":true}");
  const auto* error = r.transport.last("domisen/living/motion/error");
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_EQUAL_STRING("{\"error\":\"cảm biến chỉ đọc, không nhận lệnh\"}", error->payload.c_str());
  TEST_ASSERT_FALSE(error->retain);
  TEST_ASSERT_EQUAL(before, r.transport.count(MOTION_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"motion\":false}", r.motion.stateJson().c_str());
}

void test_cam_bien_nhi_phan_noi_lai_gui_lai_state_hien_tai() {
  BinarySensorRig r;
  r.transport.simulateConnect();
  r.hub.loop();
  r.input.value = true;
  r.hub.loop();
  r.clock.advance(50);
  r.hub.loop();
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"motion\":true}", r.transport.last(MOTION_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(MOTION_STATE)->retain);
}

// ---- đèn màu WS2812 ----
static const char* MOOD_CMD = "domisen/living/mood-light/cmd";
static const char* MOOD_STATE = "domisen/living/mood-light/state";

struct MoodRig {
  FakeTransport transport;
  FakeClock clock;
  FakePixelStrip strip;
  ColorLight mood{"living/mood-light", strip};
  Hub hub{"comfort", transport, clock, 5000};

  MoodRig() {
    strip.setColor(9, 9, 9);  // giả sử thanh đang sáng, begin() phải đưa về tắt
    hub.add(mood);
    hub.begin();
  }

  void expectStrip(uint8_t r, uint8_t g, uint8_t b) {
    TEST_ASSERT_EQUAL_UINT8(r, strip.last.r);
    TEST_ASSERT_EQUAL_UINT8(g, strip.last.g);
    TEST_ASSERT_EQUAL_UINT8(b, strip.last.b);
  }

  void expectState(const char* json) {
    const auto* state = transport.last(MOOD_STATE);
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL_STRING(json, state->payload.c_str());
  }
};

void test_den_mau_tat_luc_khoi_dong() {
  MoodRig r;
  r.expectStrip(0, 0, 0);
  r.transport.simulateConnect();
  r.expectState("{\"on\":false,\"color\":\"#FFB46B\",\"level\":100}");
  TEST_ASSERT_TRUE(r.transport.last(MOOD_STATE)->retain);
  TEST_ASSERT_EQUAL_STRING("domisen/living/mood-light/cmd", r.transport.subscriptions[1].c_str());
}

// Dòng 1 của bảng lệnh: {"on":true} bật, dùng màu và mức đã nhớ (mặc định #FFB46B, mức 100)
void test_den_mau_bang_lenh_on_true() {
  MoodRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(MOOD_CMD, "{\"on\":true}");
  r.expectStrip(255, 180, 107);
  r.expectState("{\"on\":true,\"color\":\"#FFB46B\",\"level\":100}");
}

// Dòng 2: {"on":false} tắt, nhớ màu và mức cũ
void test_den_mau_bang_lenh_on_false_nho_mau_va_muc() {
  MoodRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(MOOD_CMD, "{\"color\":\"#FF0000\",\"level\":40}");
  r.transport.simulateMessage(MOOD_CMD, "{\"on\":false}");
  r.expectStrip(0, 0, 0);
  r.expectState("{\"on\":false,\"color\":\"#FF0000\",\"level\":40}");
  r.transport.simulateMessage(MOOD_CMD, "{\"on\":true}");  // bật lại: đúng màu và mức đã nhớ
  r.expectStrip(102, 0, 0);
  r.expectState("{\"on\":true,\"color\":\"#FF0000\",\"level\":40}");
}

// Dòng 3: {"color":"#FF0000"} đổi màu và bật nếu lệnh không kèm on
void test_den_mau_bang_lenh_color() {
  MoodRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(MOOD_CMD, "{\"color\":\"#FF0000\"}");
  r.expectStrip(255, 0, 0);
  r.expectState("{\"on\":true,\"color\":\"#FF0000\",\"level\":100}");
  // kèm on:false thì chỉ đổi màu, vẫn tắt
  r.transport.simulateMessage(MOOD_CMD, "{\"on\":false,\"color\":\"#00ff00\"}");
  r.expectStrip(0, 0, 0);
  r.expectState("{\"on\":false,\"color\":\"#00FF00\",\"level\":100}");  // trả về chữ hoa
}

// Dòng 4: {"level":40} đổi độ sáng và bật nếu lệnh không kèm on; level 0 tắt, giữ mức cũ
void test_den_mau_bang_lenh_level() {
  MoodRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(MOOD_CMD, "{\"level\":40}");
  r.expectStrip(102, 72, 43);  // #FFB46B ở mức 40
  r.expectState("{\"on\":true,\"color\":\"#FFB46B\",\"level\":40}");
  r.transport.simulateMessage(MOOD_CMD, "{\"level\":0}");
  r.expectStrip(0, 0, 0);
  r.expectState("{\"on\":false,\"color\":\"#FFB46B\",\"level\":40}");
}

void test_den_mau_lenh_sai_bao_loi_va_giu_nguyen() {
  MoodRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(MOOD_CMD, "{\"color\":\"#FF0000\",\"level\":40}");
  int stateCount = r.transport.count(MOOD_STATE);
  size_t writes = r.strip.history.size();
  const char* bad[] = {"rac", "{}", "{\"color\":\"đỏ\"}", "{\"color\":\"#GGG\"}", "{\"on\":true,\"color\":\"#FFF\"}",
                       "{\"level\":\"cao\"}"};
  for (const char* payload : bad) {
    r.transport.simulateMessage(MOOD_CMD, payload);
    const auto* err = r.transport.last("domisen/living/mood-light/error");
    TEST_ASSERT_NOT_NULL(err);
    TEST_ASSERT_FALSE(err->retain);
  }
  r.expectStrip(102, 0, 0);  // vẫn như cũ
  TEST_ASSERT_EQUAL(writes, r.strip.history.size());  // không ghi gì ra thanh LED
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(MOOD_STATE));
  TEST_ASSERT_EQUAL(6, r.transport.count("domisen/living/mood-light/error"));
}

void test_den_mau_noi_lai_gui_lai_state() {
  MoodRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(MOOD_CMD, "{\"color\":\"#FF0000\",\"level\":40}");
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  r.expectState("{\"on\":true,\"color\":\"#FF0000\",\"level\":40}");
  TEST_ASSERT_TRUE(r.transport.last(MOOD_STATE)->retain);
  r.expectStrip(102, 0, 0);  // mất mạng thì đèn giữ nguyên
}

// ---- servo hai vị trí: cửa chính (guard, GPIO4), closed 0 độ, open 90 độ ----
static const char* DOOR_CMD = "domisen/entrance/door/cmd";
static const char* DOOR_STATE = "domisen/entrance/door/state";
static const char* DOOR_ERROR = "domisen/entrance/door/error";
static const char* DOOR_ERROR_TEXT = "{\"error\":\"position phải là một trong: closed, open\"}";

struct DoorRig {
  FakeTransport transport;
  FakeClock clock;
  FakePwm pwm;
  PositionDevice door{"entrance/door", pwm, {{"closed", 0.0f}, {"open", 90.0f}}};
  Hub hub{"guard", transport, clock, 5000};

  DoorRig() {
    hub.add(door);
    hub.begin();
  }
};

void test_servo_danh_sach_rong_khong_lam_gi_va_bao_loi() {
  FakeTransport transport;
  FakeClock clock;
  FakePwm pwm;
  PositionDevice empty{"entrance/door", pwm, {}};
  Hub hub{"guard", transport, clock, 5000};
  hub.add(empty);
  hub.begin();
  transport.simulateConnect();
  transport.simulateMessage("domisen/entrance/door/cmd", "{\"position\":\"open\"}");
  TEST_ASSERT_NOT_NULL(transport.last(DOOR_ERROR));
  TEST_ASSERT_EQUAL_STRING("{}", transport.last(DOOR_STATE)->payload.c_str());
}

void test_cua_khoi_dong_o_vi_tri_dau_tien() {
  DoorRig r;
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.025f, r.pwm.last);  // closed = 0 độ
  r.transport.simulateConnect();
  const auto* state = r.transport.last(DOOR_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"closed\"}", state->payload.c_str());
  TEST_ASSERT_TRUE(state->retain);
}

void test_cua_lenh_hop_le_chuyen_va_bao_state() {
  DoorRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(DOOR_CMD, "{\"position\":\"open\"}");
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0725f, r.pwm.last);  // open = 90 độ
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"open\"}", r.transport.last(DOOR_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(DOOR_STATE)->retain);
  r.transport.simulateMessage(DOOR_CMD, "{\"position\":\"closed\"}");
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.025f, r.pwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"closed\"}", r.transport.last(DOOR_STATE)->payload.c_str());
}

void test_cua_lenh_sai_bao_loi_va_giu_nguyen() {
  DoorRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(DOOR_CMD, "{\"position\":\"open\"}");
  int stateCount = r.transport.count(DOOR_STATE);
  size_t moves = r.pwm.history.size();
  r.transport.simulateMessage(DOOR_CMD, "{\"position\":\"middle\"}");  // tên không có trong danh sách
  const auto* err = r.transport.last(DOOR_ERROR);
  TEST_ASSERT_NOT_NULL(err);
  TEST_ASSERT_FALSE(err->retain);
  TEST_ASSERT_EQUAL_STRING(DOOR_ERROR_TEXT, err->payload.c_str());
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0725f, r.pwm.last);  // vẫn ở open
  TEST_ASSERT_EQUAL(moves, r.pwm.history.size());           // servo không động
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(DOOR_STATE));
}

void test_cua_noi_lai_gui_lai_state() {
  DoorRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(DOOR_CMD, "{\"position\":\"open\"}");
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"open\"}", r.transport.last(DOOR_STATE)->payload.c_str());
}

// Cửa và rèm dùng chung một lớp, chỉ khác id, PwmOutput và danh sách vị trí.
void test_cua_va_rem_chung_mot_lop() {
  FakeTransport transport;
  FakeClock clock;
  FakePwm doorPwm, curtainPwm;
  PositionDevice door("entrance/door", doorPwm, {{"closed", 0.0f}, {"open", 90.0f}});
  PositionDevice curtain("living/curtain", curtainPwm, {{"open", 0.0f}, {"closed", 90.0f}});
  Hub hub{"comfort", transport, clock, 5000};
  hub.add(door);
  hub.add(curtain);
  hub.begin();
  transport.simulateConnect();
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.025f, doorPwm.last);     // cửa: closed = 0 độ
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.025f, curtainPwm.last);  // rèm: open = 0 độ, khởi động ở open
  transport.simulateMessage("domisen/entrance/door/cmd", "{\"position\":\"open\"}");
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0725f, doorPwm.last);
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.025f, curtainPwm.last);  // rèm không đổi
  transport.simulateMessage("domisen/living/curtain/cmd", "{\"position\":\"closed\"}");
  TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.0725f, curtainPwm.last);  // rèm: closed = 90 độ
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"closed\"}", transport.last("domisen/living/curtain/state")->payload.c_str());
  TEST_ASSERT_EQUAL_STRING("{\"position\":\"open\"}", transport.last(DOOR_STATE)->payload.c_str());
}

// ---- đầu đọc thẻ (sự kiện, không phải trạng thái) ----
static const char* CARD_CMD = "domisen/entrance/card-reader/cmd";
static const char* CARD_STATE = "domisen/entrance/card-reader/state";
static const char* CARD_ERROR = "domisen/entrance/card-reader/error";

struct CardRig {
  FakeTransport transport;
  FakeClock clock;
  FakeCardReader reader;
  CardReaderDevice card{"entrance/card-reader", reader};
  Hub hub{"guard", transport, clock, 5000};

  CardRig() {
    hub.add(card);
    hub.begin();
    transport.simulateConnect();
  }
};

void test_the_su_kien_dau_gui_khong_retain() {
  CardRig r;
  r.reader.push("A3F2197C");
  r.hub.loop();
  const auto* state = r.transport.last(CARD_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"uid\":\"A3F2197C\"}", state->payload.c_str());
  TEST_ASSERT_FALSE(state->retain);
}

void test_the_noi_broker_khong_gui_state_the() {
  CardRig r;  // vừa nối broker: không có tin card-reader/state nào
  TEST_ASSERT_EQUAL(0, r.transport.count(CARD_STATE));
  r.reader.push("A3F2197C");
  r.hub.loop();
  TEST_ASSERT_EQUAL(1, r.transport.count(CARD_STATE));
}

void test_the_noi_lai_khong_gui_lai_uid_cu() {
  CardRig r;
  r.reader.push("A3F2197C");
  r.hub.loop();
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL(1, r.transport.count(CARD_STATE));  // chỉ lần quẹt, không gửi lại lúc nối lại
}

void test_the_giu_tren_dau_doc_khong_lap_trong_2_giay() {
  CardRig r;
  r.reader.push("A3F2197C");
  r.hub.loop();  // t = 0: gửi
  for (int i = 0; i < 3; i++) {
    r.clock.advance(500);
    r.reader.push("A3F2197C");  // đầu đọc vẫn báo cùng thẻ
    r.hub.loop();               // t = 500, 1000, 1500
  }
  r.clock.advance(499);
  r.reader.push("A3F2197C");
  r.hub.loop();  // t = 1999
  TEST_ASSERT_EQUAL(1, r.transport.count(CARD_STATE));
}

void test_the_giu_tren_dau_doc_gui_lai_sau_dung_2_giay() {
  CardRig r;
  r.reader.push("A3F2197C");
  r.hub.loop();  // t = 0
  r.clock.advance(1999);
  r.reader.push("A3F2197C");
  r.hub.loop();
  TEST_ASSERT_EQUAL(1, r.transport.count(CARD_STATE));
  r.clock.advance(1);
  r.reader.push("A3F2197C");
  r.hub.loop();  // t = 2000
  TEST_ASSERT_EQUAL(2, r.transport.count(CARD_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"uid\":\"A3F2197C\"}", r.transport.last(CARD_STATE)->payload.c_str());
}

void test_the_khac_gui_ngay() {
  CardRig r;
  r.reader.push("A3F2197C");
  r.hub.loop();
  r.clock.advance(10);
  r.reader.push("04B1C2D3E4F5A6");
  r.hub.loop();
  TEST_ASSERT_EQUAL(2, r.transport.count(CARD_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"uid\":\"04B1C2D3E4F5A6\"}", r.transport.last(CARD_STATE)->payload.c_str());
}

void test_the_roi_di_roi_quet_lai_gui_ngay() {
  CardRig r;
  r.reader.push("A3F2197C");
  r.hub.loop();
  r.clock.advance(100);
  r.reader.pushEmpty();  // thẻ rời đi
  r.hub.loop();
  TEST_ASSERT_EQUAL(1, r.transport.count(CARD_STATE));
  r.clock.advance(100);
  r.reader.push("A3F2197C");  // quẹt lại, mới 200 ms
  r.hub.loop();
  TEST_ASSERT_EQUAL(2, r.transport.count(CARD_STATE));
}

void test_the_khong_co_the_thi_khong_gui() {
  CardRig r;
  r.reader.pushEmpty();
  r.hub.loop();
  r.clock.advance(5000);
  r.hub.loop();
  TEST_ASSERT_EQUAL(0, r.transport.count(CARD_STATE));
}

void test_the_mat_ket_noi_thi_khong_doc() {
  CardRig r;
  r.transport.simulateDisconnect();
  r.reader.push("A3F2197C");
  r.hub.loop();
  TEST_ASSERT_EQUAL(0, r.reader.polls);
}

void test_the_chi_doc_khong_nhan_lenh() {
  CardRig r;
  r.transport.simulateMessage(CARD_CMD, "{\"on\":true}");
  const auto* err = r.transport.last(CARD_ERROR);
  TEST_ASSERT_NOT_NULL(err);
  TEST_ASSERT_FALSE(err->retain);
  TEST_ASSERT_EQUAL_STRING("{\"error\":\"đầu đọc thẻ chỉ đọc, không nhận lệnh\"}", err->payload.c_str());
  TEST_ASSERT_EQUAL(0, r.transport.count(CARD_STATE));
}

// Thêm đầu đọc thẻ vào Hub không đổi hành vi của đèn, quạt, DHT22: vẫn retain và vẫn gửi lại lúc nối lại.
void test_the_khong_doi_hanh_vi_cua_thiet_bi_khac() {
  FakeTransport transport;
  FakeClock clock;
  FakePwm pwm;
  FakeDigitalOutput relay;
  FakeClimateSensor sensor;
  FakeCardReader reader;
  sensor.next = core::ClimateReading{27.5f, 62.0f};
  DimmableLight light{"living/ceiling-light", pwm};
  SwitchDevice fan{"living/fan", relay};
  ClimateDevice climate{"living/climate", sensor, 10000};
  CardReaderDevice card{"entrance/card-reader", reader};
  Hub hub{"guard", transport, clock, 5000};
  hub.add(light);
  hub.add(fan);
  hub.add(climate);
  hub.add(card);
  hub.begin();
  transport.simulateConnect();
  hub.loop();
  transport.simulateMessage(FAN_CMD, "{\"on\":true}");
  transport.published.clear();
  transport.simulateDisconnect();
  transport.simulateConnect();
  TEST_ASSERT_EQUAL(1, transport.count(CEILING_STATE));
  TEST_ASSERT_TRUE(transport.last(CEILING_STATE)->retain);
  TEST_ASSERT_EQUAL(1, transport.count(FAN_STATE));
  TEST_ASSERT_TRUE(transport.last(FAN_STATE)->retain);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", transport.last(FAN_STATE)->payload.c_str());
  TEST_ASSERT_EQUAL(1, transport.count(CLIMATE_STATE));
  TEST_ASSERT_TRUE(transport.last(CLIMATE_STATE)->retain);
  TEST_ASSERT_EQUAL(0, transport.count(CARD_STATE));
}

// ---- còi và quạt hút trên board guard ----
static const char* ALARM_CMD = "domisen/house/alarm/cmd";
static const char* ALARM_STATE = "domisen/house/alarm/state";
static const char* EXHAUST_FAN_CMD = "domisen/kitchen/exhaust-fan/cmd";
static const char* EXHAUST_FAN_STATE = "domisen/kitchen/exhaust-fan/state";

struct GuardSwitchRig {
  FakeTransport transport;
  FakeClock clock;
  FakeDigitalOutput alarmOutput;
  FakeDigitalOutput exhaustFanRelay;
  SwitchDevice alarm{"house/alarm", alarmOutput};
  SwitchDevice exhaustFan{"kitchen/exhaust-fan", exhaustFanRelay};
  Hub hub{"guard", transport, clock, 5000};

  GuardSwitchRig() {
    alarmOutput.last = true;
    exhaustFanRelay.last = true;
    hub.add(alarm);
    hub.add(exhaustFan);
    hub.begin();
  }
};

void test_guard_coi_va_quat_hut_dang_ky_dung_topic_khoi_dong_tat_va_gui_state() {
  GuardSwitchRig r;
  TEST_ASSERT_EQUAL(3, r.transport.subscriptions.size());
  TEST_ASSERT_EQUAL_STRING("domisen/node/guard/ping", r.transport.subscriptions[0].c_str());
  TEST_ASSERT_EQUAL_STRING(ALARM_CMD, r.transport.subscriptions[1].c_str());
  TEST_ASSERT_EQUAL_STRING(EXHAUST_FAN_CMD, r.transport.subscriptions[2].c_str());
  TEST_ASSERT_FALSE(r.alarmOutput.last);
  TEST_ASSERT_FALSE(r.exhaustFanRelay.last);

  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(ALARM_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(ALARM_STATE)->retain);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(EXHAUST_FAN_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(EXHAUST_FAN_STATE)->retain);
}

void test_guard_coi_va_quat_hut_bat_tat_va_gui_state_dung_topic() {
  GuardSwitchRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(ALARM_CMD, "{\"on\":true}");
  TEST_ASSERT_TRUE(r.alarmOutput.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(ALARM_STATE)->payload.c_str());
  r.transport.simulateMessage(ALARM_CMD, "{\"on\":false}");
  TEST_ASSERT_FALSE(r.alarmOutput.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(ALARM_STATE)->payload.c_str());

  r.transport.simulateMessage(EXHAUST_FAN_CMD, "{\"on\":true}");
  TEST_ASSERT_TRUE(r.exhaustFanRelay.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(EXHAUST_FAN_STATE)->payload.c_str());
  r.transport.simulateMessage(EXHAUST_FAN_CMD, "{\"on\":false}");
  TEST_ASSERT_FALSE(r.exhaustFanRelay.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(EXHAUST_FAN_STATE)->payload.c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_cua_khoi_dong_o_vi_tri_dau_tien);
  RUN_TEST(test_servo_danh_sach_rong_khong_lam_gi_va_bao_loi);
  RUN_TEST(test_cua_lenh_hop_le_chuyen_va_bao_state);
  RUN_TEST(test_cua_lenh_sai_bao_loi_va_giu_nguyen);
  RUN_TEST(test_cua_noi_lai_gui_lai_state);
  RUN_TEST(test_cua_va_rem_chung_mot_lop);
  RUN_TEST(test_begin_dang_ky_topic_lenh_va_ping);
  RUN_TEST(test_den_tat_luc_khoi_dong);
  RUN_TEST(test_noi_broker_bao_online_va_trang_thai_retain);
  RUN_TEST(test_lenh_dieu_khien_pwm_va_bao_lai_trang_thai);
  RUN_TEST(test_quat_toc_do_tat_luc_khoi_dong);
  RUN_TEST(test_quat_toc_do_lenh_cu_on_van_chay);
  RUN_TEST(test_quat_toc_do_level_ra_duty_tuyen_tinh_tu_min);
  RUN_TEST(test_quat_toc_do_dung_yen_len_muc_thap_thi_kick_start);
  RUN_TEST(test_quat_toc_do_bat_lai_sau_khi_tat_thi_lai_kick);
  RUN_TEST(test_quat_toc_do_doi_muc_trong_luc_da_thi_doi_muc_dich);
  RUN_TEST(test_quat_toc_do_tat_trong_luc_da_thi_dung_han_va_khong_chay_lai);
  RUN_TEST(test_quat_toc_do_len_het_ga_thi_khong_can_kick);
  RUN_TEST(test_quat_toc_do_lenh_sai_bao_loi_va_giu_nguyen);
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
  RUN_TEST(test_quat_tat_luc_khoi_dong);
  RUN_TEST(test_quat_bat_tat_va_bao_lai_state);
  RUN_TEST(test_quat_lenh_sai_bao_loi_va_giu_nguyen);
  RUN_TEST(test_quat_noi_lai_gui_lai_state);
  RUN_TEST(test_cam_bien_nhi_phan_chua_doc_gui_null_va_lan_dau_nhan_ngay);
  RUN_TEST(test_cam_bien_nhi_phan_chong_nhieu_va_nhan_gia_tri_on_dinh_sau_50ms);
  RUN_TEST(test_cam_bien_nhi_phan_bo_qua_nhieu_ngan_hon_50ms);
  RUN_TEST(test_cam_bien_nhi_phan_active_low_dao_muc);
  RUN_TEST(test_cam_bien_nhi_phan_chi_doc_va_khong_doi_state_khi_nhan_lenh);
  RUN_TEST(test_cam_bien_nhi_phan_noi_lai_gui_lai_state_hien_tai);
  RUN_TEST(test_den_mau_tat_luc_khoi_dong);
  RUN_TEST(test_den_mau_bang_lenh_on_true);
  RUN_TEST(test_den_mau_bang_lenh_on_false_nho_mau_va_muc);
  RUN_TEST(test_den_mau_bang_lenh_color);
  RUN_TEST(test_den_mau_bang_lenh_level);
  RUN_TEST(test_den_mau_lenh_sai_bao_loi_va_giu_nguyen);
  RUN_TEST(test_den_mau_noi_lai_gui_lai_state);
  RUN_TEST(test_the_su_kien_dau_gui_khong_retain);
  RUN_TEST(test_the_noi_broker_khong_gui_state_the);
  RUN_TEST(test_the_noi_lai_khong_gui_lai_uid_cu);
  RUN_TEST(test_the_giu_tren_dau_doc_khong_lap_trong_2_giay);
  RUN_TEST(test_the_giu_tren_dau_doc_gui_lai_sau_dung_2_giay);
  RUN_TEST(test_the_khac_gui_ngay);
  RUN_TEST(test_the_roi_di_roi_quet_lai_gui_ngay);
  RUN_TEST(test_the_khong_co_the_thi_khong_gui);
  RUN_TEST(test_the_mat_ket_noi_thi_khong_doc);
  RUN_TEST(test_the_chi_doc_khong_nhan_lenh);
  RUN_TEST(test_the_khong_doi_hanh_vi_cua_thiet_bi_khac);
  RUN_TEST(test_guard_coi_va_quat_hut_dang_ky_dung_topic_khoi_dong_tat_va_gui_state);
  RUN_TEST(test_guard_coi_va_quat_hut_bat_tat_va_gui_state_dung_topic);
  return UNITY_END();
}
