// Hub, đèn, cảm biến, quạt, servo và đầu đọc thẻ, chạy với bản giả: không cần board, không cần mạng.
#include <shared/adapters_fake/FakeCardReader.h>
#include <shared/adapters_fake/FakeAnalogInput.h>
#include <shared/adapters_fake/FakeClimateSensor.h>
#include <shared/adapters_fake/FakeClock.h>
#include <shared/adapters_fake/FakeDigitalInput.h>
#include <shared/adapters_fake/FakeDigitalOutput.h>
#include <shared/adapters_fake/FakePixelStrip.h>
#include <shared/adapters_fake/FakePwm.h>
#include <shared/adapters_fake/FakeTextDisplay.h>
#include <shared/adapters_fake/FakeTransport.h>
#include <shared/services/BinarySensorDevice.h>
#include <shared/services/CardReaderDevice.h>
#include <shared/services/ClimateDevice.h>
#include <shared/services/GasSafety.h>
#include <shared/services/GasSensorDevice.h>
#include <shared/services/LightSensorDevice.h>
#include <shared/services/ColorLight.h>
#include <shared/services/DimmableLight.h>
#include <shared/services/DisplayDevice.h>
#include <shared/services/Hub.h>
#include <shared/services/PositionDevice.h>
#include <shared/services/SpeedFan.h>
#include <shared/services/SwitchDevice.h>
#include <shared/services/TouchToggle.h>
#include <unity.h>

using namespace domisen;
using adapters::fake::FakeCardReader;
using adapters::fake::FakeAnalogInput;
using adapters::fake::FakeClimateSensor;
using adapters::fake::FakeClock;
using adapters::fake::FakeDigitalInput;
using adapters::fake::FakeDigitalOutput;
using adapters::fake::FakePixelStrip;
using adapters::fake::FakePwm;
using adapters::fake::FakeTextDisplay;
using adapters::fake::FakeTransport;
using services::BinarySensorDevice;
using services::CardReaderDevice;
using services::ClimateDevice;
using services::GasSafety;
using services::GasSensorDevice;
using services::LightSensorDevice;
using services::ColorLight;
using services::DimmableLight;
using services::DisplayDevice;
using services::Hub;
using services::PositionDevice;
using services::SpeedFan;
using services::SwitchDevice;
using services::TouchToggle;

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

// ---- màn hình OLED ----
static const char* DISPLAY_CMD = "domisen/entrance/display/cmd";
static const char* DISPLAY_STATE = "domisen/entrance/display/state";

struct DisplayRig {
  FakeTransport transport;
  FakeClock clock;
  FakeTextDisplay screen;
  DisplayDevice display{"entrance/display", screen};
  Hub hub{"guard", transport, clock, 5000};

  DisplayRig() {
    hub.add(display);
    hub.begin();
  }
};

// ---- cảm biến gas MQ-2 (docs/mqtt-topics.md, mục "Cảm biến gas MQ-2") ----
static const char* GAS_CMD = "domisen/kitchen/gas/cmd";
static const char* GAS_STATE = "domisen/kitchen/gas/state";

// Mẫu đọc lần lượt theo danh sách rồi quay vòng, để thử việc lấy trung bình.
struct SequenceAnalogInput : core::AnalogInput {
  uint16_t read() override {
    const uint16_t v = values[next % values.size()];
    ++next;
    return v;
  }
  std::vector<uint16_t> values;
  size_t next = 0;
};

struct GasRig {
  FakeTransport transport;
  FakeClock clock;
  FakeAnalogInput input;
  GasSensorDevice gas;
  Hub hub{"guard", transport, clock, 5000};

  // Ngưỡng bật 2000, ngưỡng tắt 1800, làm nóng 60 s, lấy mẫu 1 s, gửi 5 s, 8 mẫu (giá trị mặc định của Config).
  explicit GasRig(int offRaw = 1800) : gas("kitchen/gas", input, clock, {.onRaw = 2000, .offRaw = offRaw}) {
    hub.add(gas);
    hub.begin();
  }
  void connectAndWarmUp() {
    transport.simulateConnect();
    clock.advance(60000);
    hub.loop();
  }
  void step(uint32_t ms = 1000) {
    clock.advance(ms);
    hub.loop();
  }
};

void test_gas_dang_lam_nong_state_null_va_chua_doc() {
  GasRig r;
  r.transport.simulateConnect();
  r.clock.advance(59999);
  r.hub.loop();
  TEST_ASSERT_EQUAL(0, r.input.reads);
  const auto* state = r.transport.last(GAS_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":null,\"alarm\":false}", state->payload.c_str());
  TEST_ASSERT_TRUE(state->retain);
}

void test_gas_het_lam_nong_doc_8_mau_va_gui_ngay() {
  GasRig r;
  r.input.value = 500;
  r.connectAndWarmUp();
  TEST_ASSERT_EQUAL(8, r.input.reads);
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":500,\"alarm\":false}", r.transport.last(GAS_STATE)->payload.c_str());
}

void test_gas_ppm_raw_la_trung_binh_8_mau() {
  FakeTransport transport;
  FakeClock clock;
  SequenceAnalogInput input;
  input.values = {0, 100, 200, 300, 400, 500, 600, 700};  // trung bình 350
  GasSensorDevice gas("kitchen/gas", input, clock, {.onRaw = 2000, .offRaw = 1800});
  Hub hub{"guard", transport, clock, 5000};
  hub.add(gas);
  hub.begin();
  transport.simulateConnect();
  clock.advance(60000);
  hub.loop();
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":350,\"alarm\":false}", transport.last(GAS_STATE)->payload.c_str());
}

void test_gas_alarm_bat_dung_nguong_va_gui_ngay() {
  GasRig r;
  r.input.value = 500;
  r.connectAndWarmUp();
  const int before = r.transport.count(GAS_STATE);
  r.input.value = 1999;
  r.step();
  TEST_ASSERT_EQUAL(before, r.transport.count(GAS_STATE));  // chưa tới ngưỡng, chưa tới chu kỳ gửi 5 s
  r.input.value = 2000;
  r.step();
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(GAS_STATE));  // alarm đổi thì gửi ngay
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":2000,\"alarm\":true}", r.transport.last(GAS_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.gas.alarm());
}

void test_gas_alarm_co_tre_khong_nhap_nhay_quanh_nguong() {
  GasRig r;
  r.input.value = 2100;
  r.connectAndWarmUp();
  TEST_ASSERT_TRUE(r.gas.alarm());
  r.input.value = 1900;  // giữa hai ngưỡng: vẫn báo động
  r.step();
  TEST_ASSERT_TRUE(r.gas.alarm());
  r.input.value = 1801;
  r.step();
  TEST_ASSERT_TRUE(r.gas.alarm());
  const int before = r.transport.count(GAS_STATE);
  r.input.value = 1799;  // xuống dưới ngưỡng tắt
  r.step();
  TEST_ASSERT_FALSE(r.gas.alarm());
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(GAS_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":1799,\"alarm\":false}", r.transport.last(GAS_STATE)->payload.c_str());
  r.input.value = 1900;  // lên lại giữa hai ngưỡng: chưa tự bật
  r.step();
  TEST_ASSERT_FALSE(r.gas.alarm());
}

void test_gas_lay_mau_moi_giay_va_gui_moi_5_giay_khi_alarm_khong_doi() {
  GasRig r;
  r.input.value = 500;
  r.connectAndWarmUp();
  const int reads = r.input.reads;
  const int sent = r.transport.count(GAS_STATE);
  r.step(500);
  TEST_ASSERT_EQUAL(reads, r.input.reads);  // chưa đủ 1 giây
  r.step(500);
  TEST_ASSERT_EQUAL(reads + 8, r.input.reads);
  r.step();
  r.step();
  r.step();
  TEST_ASSERT_EQUAL(sent, r.transport.count(GAS_STATE));  // mới 4 giây sau lần gửi đầu
  r.input.value = 520;
  r.step();  // đúng 5 giây sau lần gửi đầu
  TEST_ASSERT_EQUAL(sent + 1, r.transport.count(GAS_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":520,\"alarm\":false}", r.transport.last(GAS_STATE)->payload.c_str());
}

void test_gas_chi_doc_khong_nhan_lenh() {
  GasRig r;
  r.input.value = 500;
  r.connectAndWarmUp();
  const int stateCount = r.transport.count(GAS_STATE);
  r.transport.simulateMessage(GAS_CMD, "{\"on\":true}");
  const auto* error = r.transport.last("domisen/kitchen/gas/error");
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_EQUAL_STRING("{\"error\":\"cảm biến chỉ đọc, không nhận lệnh\"}", error->payload.c_str());
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(GAS_STATE));
}

void test_gas_noi_lai_gui_lai_state_hien_tai() {
  GasRig r;
  r.input.value = 2300;
  r.connectAndWarmUp();
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":2300,\"alarm\":true}", r.transport.last(GAS_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(GAS_STATE)->retain);
}

void test_gas_nguong_tat_cao_hon_nguong_bat_bi_ep_bang_nguong_bat() {
  GasRig r(2500);  // sai cấu hình: ngưỡng tắt cao hơn ngưỡng bật
  r.input.value = 2000;
  r.connectAndWarmUp();
  TEST_ASSERT_TRUE(r.gas.alarm());
  r.input.value = 1999;
  r.step();
  TEST_ASSERT_FALSE(r.gas.alarm());  // không kẹt ở trạng thái báo động
}

// ---- quy tắc an toàn khi có gas (docs/mqtt-topics.md, mục "Còi và quạt hút") ----
static const char* BUZZER_CMD = "domisen/house/alarm/cmd";
static const char* BUZZER_STATE = "domisen/house/alarm/state";
static const char* FAN_HOOD_CMD = "domisen/kitchen/exhaust-fan/cmd";
static const char* FAN_HOOD_STATE = "domisen/kitchen/exhaust-fan/state";
static const char* FAN_HOOD_ERROR = "domisen/kitchen/exhaust-fan/error";

void test_switch_setOn_tu_board_doi_output_va_bao_hub_gui_state_mot_lan() {
  FakeDigitalOutput out;
  SwitchDevice device("house/alarm", out);
  device.begin();
  TEST_ASSERT_FALSE(device.tick(0));
  device.setOn(true);
  TEST_ASSERT_TRUE(out.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", device.stateJson().c_str());
  TEST_ASSERT_TRUE(device.tick(1));
  TEST_ASSERT_FALSE(device.tick(2));
  device.setOn(true);  // không đổi thì không cần gửi lại
  TEST_ASSERT_FALSE(device.tick(3));
}

void test_switch_khoa_tu_choi_lenh_tat_nhung_nhan_lenh_bat_va_mo_khoa_thi_tat_duoc() {
  FakeDigitalOutput out;
  SwitchDevice device("kitchen/exhaust-fan", out);
  device.begin();
  device.setOn(true);
  device.lockOn("đang có gas");
  std::string error;
  TEST_ASSERT_FALSE(device.onCommand("{\"on\":false}", error));
  TEST_ASSERT_EQUAL_STRING("đang có gas", error.c_str());
  TEST_ASSERT_TRUE(out.last);
  TEST_ASSERT_TRUE(device.onCommand("{\"on\":true}", error));
  device.unlock();
  TEST_ASSERT_TRUE(device.onCommand("{\"on\":false}", error));
  TEST_ASSERT_FALSE(out.last);
}

struct GasSafetyRig {
  FakeTransport transport;
  FakeClock clock;
  FakeAnalogInput input;
  FakeDigitalOutput buzzerOut;
  FakeDigitalOutput fanOut;
  SwitchDevice buzzer;
  SwitchDevice fan;
  GasSensorDevice gas;
  GasSafety safety;
  Hub hub{"guard", transport, clock, 5000};

  // Ngưỡng bật 2000, ngưỡng tắt 1800 (xem GasRig).
  GasSafetyRig()
      : buzzer("house/alarm", buzzerOut),
        fan("kitchen/exhaust-fan", fanOut),
        gas("kitchen/gas", input, clock, {.onRaw = 2000, .offRaw = 1800}),
        safety(gas, buzzer, fan) {
    hub.add(buzzer);
    hub.add(fan);
    hub.add(gas);
    hub.begin();
    safety.begin();
  }
  void warmUpAndStep(uint16_t value) {
    input.value = value;
    clock.advance(60000);
    hub.loop();
    hub.loop();  // vòng sau gửi state của hai công tắc vừa bị code trên board đổi
  }
  void step(uint16_t value, uint32_t ms = 1000) {
    input.value = value;
    clock.advance(ms);
    hub.loop();
    hub.loop();
  }
};

void test_gas_an_toan_co_gas_bat_con_va_quat_hut_va_gui_state() {
  GasSafetyRig r;
  r.transport.simulateConnect();
  r.warmUpAndStep(2500);
  TEST_ASSERT_TRUE(r.gas.alarm());
  TEST_ASSERT_TRUE(r.buzzerOut.last);
  TEST_ASSERT_TRUE(r.fanOut.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(BUZZER_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(BUZZER_STATE)->retain);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(FAN_HOOD_STATE)->payload.c_str());
}

void test_gas_an_toan_het_gas_tat_ca_hai_va_bo_khoa() {
  GasSafetyRig r;
  r.transport.simulateConnect();
  r.warmUpAndStep(2500);
  r.step(1900);  // gas loãng dần nhưng chưa xuống dưới ngưỡng tắt 1800: vẫn báo động
  TEST_ASSERT_TRUE(r.fanOut.last);
  r.step(1799);
  TEST_ASSERT_FALSE(r.gas.alarm());
  TEST_ASSERT_FALSE(r.buzzerOut.last);
  TEST_ASSERT_FALSE(r.fanOut.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(BUZZER_STATE)->payload.c_str());
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(FAN_HOOD_STATE)->payload.c_str());
  r.transport.simulateMessage(FAN_HOOD_CMD, "{\"on\":true}");  // hết gas: quạt hút lại điều khiển như thường
  r.transport.simulateMessage(FAN_HOOD_CMD, "{\"on\":false}");
  TEST_ASSERT_FALSE(r.fanOut.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(FAN_HOOD_STATE)->payload.c_str());
}

void test_gas_an_toan_dang_bao_dong_tu_choi_lenh_tat_quat_hut() {
  GasSafetyRig r;
  r.transport.simulateConnect();
  r.warmUpAndStep(2500);
  const int stateCount = r.transport.count(FAN_HOOD_STATE);
  r.transport.simulateMessage(FAN_HOOD_CMD, "{\"on\":false}");
  TEST_ASSERT_TRUE(r.fanOut.last);
  const auto* error = r.transport.last(FAN_HOOD_ERROR);
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_FALSE(error->retain);
  TEST_ASSERT_EQUAL_STRING("{\"error\":\"đang có gas, không tắt quạt hút được\"}", error->payload.c_str());
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(FAN_HOOD_STATE));
  r.transport.simulateMessage(FAN_HOOD_CMD, "{\"on\":true}");  // lệnh bật vẫn nhận
  TEST_ASSERT_TRUE(r.fanOut.last);
}

void test_gas_an_toan_tat_tieng_coi_khong_tu_bat_lai_cho_toi_lan_bao_dong_sau() {
  GasSafetyRig r;
  r.transport.simulateConnect();
  r.warmUpAndStep(2500);
  TEST_ASSERT_TRUE(r.buzzerOut.last);
  r.transport.simulateMessage(BUZZER_CMD, "{\"on\":false}");  // tắt tiếng được
  TEST_ASSERT_FALSE(r.buzzerOut.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false}", r.transport.last(BUZZER_STATE)->payload.c_str());
  r.step(2600);
  r.step(2700);
  TEST_ASSERT_TRUE(r.gas.alarm());
  TEST_ASSERT_FALSE(r.buzzerOut.last);  // gas vẫn còn nhưng còi không tự bật lại
  TEST_ASSERT_TRUE(r.fanOut.last);      // quạt hút vẫn chạy
  r.step(100);                          // hết gas
  TEST_ASSERT_FALSE(r.gas.alarm());
  TEST_ASSERT_FALSE(r.fanOut.last);
  r.step(2500);                         // báo động lần sau: còi kêu lại
  TEST_ASSERT_TRUE(r.gas.alarm());
  TEST_ASSERT_TRUE(r.buzzerOut.last);
  TEST_ASSERT_TRUE(r.fanOut.last);
}

void test_gas_an_toan_mat_ket_noi_van_bat_con_va_quat_hut_roi_gui_state_khi_noi_lai() {
  GasSafetyRig r;
  r.transport.simulateConnect();
  r.transport.simulateDisconnect();  // mất mạng, quy tắc an toàn vẫn phải chạy
  const int publishedBefore = static_cast<int>(r.transport.published.size());
  r.warmUpAndStep(2500);
  TEST_ASSERT_TRUE(r.input.reads > 0);
  TEST_ASSERT_TRUE(r.gas.alarm());
  TEST_ASSERT_TRUE(r.buzzerOut.last);
  TEST_ASSERT_TRUE(r.fanOut.last);
  TEST_ASSERT_EQUAL(publishedBefore, static_cast<int>(r.transport.published.size()));  // offline: chưa gửi được gì
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(BUZZER_STATE)->payload.c_str());
  TEST_ASSERT_EQUAL_STRING("{\"on\":true}", r.transport.last(FAN_HOOD_STATE)->payload.c_str());
  TEST_ASSERT_EQUAL_STRING("{\"ppm_raw\":2500,\"alarm\":true}", r.transport.last(GAS_STATE)->payload.c_str());
}

void test_gas_an_toan_chi_thiet_bi_an_toan_chay_khi_mat_ket_noi() {
  // Cảm biến khí hậu không chạy lúc mất kết nối (test_khi_hau_mat_ket_noi_thi_khong_do); chỉ thiết bị runsOffline() mới được tick.
  GasSafetyRig r;
  r.transport.simulateConnect();
  r.transport.simulateDisconnect();
  r.input.value = 100;
  r.clock.advance(60000);
  r.hub.loop();
  TEST_ASSERT_EQUAL(8, r.input.reads);
}

// ---- cảm biến ánh sáng (docs/mqtt-topics.md, mục "Cảm biến ánh sáng") ----
static const char* LIGHT_SENSOR_CMD = "domisen/living/light-sensor/cmd";
static const char* LIGHT_SENSOR_STATE = "domisen/living/light-sensor/state";

struct LightSensorRig {
  FakeTransport transport;
  FakeClock clock;
  FakeAnalogInput input;
  LightSensorDevice sensor;
  Hub hub{"comfort", transport, clock, 5000};

  // dark bật khi dưới 800, tắt khi đạt 1200; lấy mẫu 1 s, gửi 10 s, 8 mẫu (mặc định của Config).
  explicit LightSensorRig(bool aoFallsWhenBright = false, int brightAtOrAbove = 1200)
      : sensor("living/light-sensor", input,
               {.darkBelowRaw = 800, .brightAtOrAboveRaw = brightAtOrAbove, .aoFallsWhenBright = aoFallsWhenBright}) {
    hub.add(sensor);
    hub.begin();
  }
  void connectAndRead(uint16_t value) {
    input.value = value;
    transport.simulateConnect();
    hub.loop();
  }
  void step(uint16_t value, uint32_t ms = 1000) {
    input.value = value;
    clock.advance(ms);
    hub.loop();
  }
};

void test_anh_sang_chua_doc_state_null() {
  LightSensorRig r;
  r.transport.simulateConnect();  // chưa tick nên chưa đọc lần nào
  const auto* state = r.transport.last(LIGHT_SENSOR_STATE);
  TEST_ASSERT_NOT_NULL(state);
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":null,\"dark\":false}", state->payload.c_str());
  TEST_ASSERT_TRUE(state->retain);
}

void test_anh_sang_doc_ngay_khi_noi_8_mau_va_gui_state() {
  LightSensorRig r;
  r.connectAndRead(2310);
  TEST_ASSERT_EQUAL(8, r.input.reads);
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":2310,\"dark\":false}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
}

void test_anh_sang_light_raw_la_trung_binh_8_mau() {
  FakeTransport transport;
  FakeClock clock;
  SequenceAnalogInput input;
  input.values = {0, 100, 200, 300, 400, 500, 600, 700};  // trung bình 350
  LightSensorDevice sensor("living/light-sensor", input, {.darkBelowRaw = 100, .brightAtOrAboveRaw = 200});
  Hub hub{"comfort", transport, clock, 5000};
  hub.add(sensor);
  hub.begin();
  transport.simulateConnect();
  hub.loop();
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":350,\"dark\":false}", transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
}

void test_anh_sang_chuan_hoa_chieu_khi_ao_giam_luc_sang() {
  LightSensorRig r(true);  // AO giảm khi sáng: đọc thô 1000 là khá sáng, light_raw = 4095 - 1000
  r.connectAndRead(1000);
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":3095,\"dark\":false}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
  r.step(4000);  // đọc thô 4000 là rất tối: light_raw = 95, dưới ngưỡng 800
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":95,\"dark\":true}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
}

void test_anh_sang_dark_bat_dung_nguong_va_gui_ngay() {
  LightSensorRig r;
  r.connectAndRead(2000);
  const int before = r.transport.count(LIGHT_SENSOR_STATE);
  r.step(800);  // đúng ngưỡng 800: chưa tối, chưa tới chu kỳ gửi 10 s
  TEST_ASSERT_EQUAL(before, r.transport.count(LIGHT_SENSOR_STATE));
  r.step(799);
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(LIGHT_SENSOR_STATE));  // dark đổi thì gửi ngay
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":799,\"dark\":true}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
}

void test_anh_sang_dark_co_tre_khong_nhap_nhay_quanh_nguong() {
  LightSensorRig r;
  r.connectAndRead(500);  // tối ngay từ lần đọc đầu
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":500,\"dark\":true}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
  const int before = r.transport.count(LIGHT_SENSOR_STATE);
  r.step(1100);  // giữa hai ngưỡng: vẫn tối, dark không đổi nên chưa gửi
  TEST_ASSERT_EQUAL(before, r.transport.count(LIGHT_SENSOR_STATE));
  r.step(1199);
  TEST_ASSERT_EQUAL(before, r.transport.count(LIGHT_SENSOR_STATE));
  r.step(1200);  // đạt ngưỡng sáng thì mới hết tối
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(LIGHT_SENSOR_STATE));
}

void test_anh_sang_dark_tat_khi_dat_nguong_sang_roi_khong_tu_bat_o_giua() {
  LightSensorRig r;
  r.connectAndRead(500);
  const int before = r.transport.count(LIGHT_SENSOR_STATE);
  r.step(1200);  // đạt ngưỡng sáng: hết tối
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(LIGHT_SENSOR_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":1200,\"dark\":false}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
  r.step(1000);  // lại giữa hai ngưỡng: chưa tự tối lại
  TEST_ASSERT_EQUAL(before + 1, r.transport.count(LIGHT_SENSOR_STATE));
}

void test_anh_sang_lay_mau_moi_giay_va_gui_moi_10_giay_khi_dark_khong_doi() {
  LightSensorRig r;
  r.connectAndRead(2000);
  const int reads = r.input.reads;
  const int sent = r.transport.count(LIGHT_SENSOR_STATE);
  r.step(2000, 500);
  TEST_ASSERT_EQUAL(reads, r.input.reads);  // chưa đủ 1 giây
  r.step(2000, 500);
  TEST_ASSERT_EQUAL(reads + 8, r.input.reads);
  for (int i = 0; i < 8; ++i) r.step(2000);  // tới giây thứ 9 sau lần gửi đầu
  TEST_ASSERT_EQUAL(sent, r.transport.count(LIGHT_SENSOR_STATE));
  r.step(2050);  // đúng 10 giây
  TEST_ASSERT_EQUAL(sent + 1, r.transport.count(LIGHT_SENSOR_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":2050,\"dark\":false}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
}

void test_anh_sang_chi_doc_khong_nhan_lenh() {
  LightSensorRig r;
  r.connectAndRead(2000);
  const int stateCount = r.transport.count(LIGHT_SENSOR_STATE);
  r.transport.simulateMessage(LIGHT_SENSOR_CMD, "{\"on\":true}");
  const auto* error = r.transport.last("domisen/living/light-sensor/error");
  TEST_ASSERT_NOT_NULL(error);
  TEST_ASSERT_EQUAL_STRING("{\"error\":\"cảm biến chỉ đọc, không nhận lệnh\"}", error->payload.c_str());
  TEST_ASSERT_EQUAL(stateCount, r.transport.count(LIGHT_SENSOR_STATE));
}

void test_anh_sang_noi_lai_gui_lai_state_hien_tai() {
  LightSensorRig r;
  r.connectAndRead(500);
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":500,\"dark\":true}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(LIGHT_SENSOR_STATE)->retain);
}

void test_anh_sang_mat_ket_noi_thi_khong_lay_mau() {
  LightSensorRig r;
  r.transport.simulateConnect();
  r.transport.simulateDisconnect();
  r.input.value = 2000;
  r.clock.advance(30000);
  r.hub.loop();
  TEST_ASSERT_EQUAL(0, r.input.reads);
}

void test_anh_sang_nguong_sang_thap_hon_nguong_toi_bi_ep_bang_nguong_toi() {
  LightSensorRig r(false, 500);  // sai cấu hình: ngưỡng sáng 500 thấp hơn ngưỡng tối 800
  r.connectAndRead(700);
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":700,\"dark\":true}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
  r.step(800);  // tới 800 thì hết tối, không kẹt
  TEST_ASSERT_EQUAL_STRING("{\"light_raw\":800,\"dark\":false}", r.transport.last(LIGHT_SENSOR_STATE)->payload.c_str());
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

void test_oled_khoi_dong_trong_va_gui_state_retain() {
  DisplayRig r;
  TEST_ASSERT_EQUAL_STRING("{\"text\":\"\"}", r.display.stateJson().c_str());
  TEST_ASSERT_EQUAL(0, r.screen.last.size());
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"text\":\"\"}", r.transport.last(DISPLAY_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(DISPLAY_STATE)->retain);
}

void test_oled_hien_thi_mot_dong_va_bon_dong_moi_dong_16_ky_tu() {
  DisplayRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(DISPLAY_CMD, "{\"text\":\"Xin chao\"}");
  TEST_ASSERT_EQUAL(1, r.screen.last.size());
  TEST_ASSERT_EQUAL_STRING("Xin chao", r.screen.last[0].c_str());
  TEST_ASSERT_EQUAL_STRING("{\"text\":\"Xin chao\"}", r.transport.last(DISPLAY_STATE)->payload.c_str());

  r.transport.simulateMessage(DISPLAY_CMD,
                              "{\"text\":\"1234567890123456\\nDong 2\\nDong 3\\nDong 4\"}");
  TEST_ASSERT_EQUAL(4, r.screen.last.size());
  TEST_ASSERT_EQUAL(16, r.screen.last[0].size());
  TEST_ASSERT_EQUAL_STRING("Dong 4", r.screen.last[3].c_str());
}

void test_oled_tu_choi_lenh_sai_va_giu_nguyen_man_hinh() {
  DisplayRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(DISPLAY_CMD, "{\"text\":\"Xin chao\"}");
  const auto oldLines = r.screen.last;
  const std::string oldState = r.display.stateJson();
  const size_t oldHistory = r.screen.history.size();

  // Mỗi lệnh sai báo đúng một lỗi riêng (không retain) và không đụng tới màn hình hay state.
  const struct {
    const char* command;
    const char* error;
  } badCommands[] = {
      {"{\"text\":\"1\\n2\\n3\\n4\\n5\"}", "{\"error\":\"text vượt quá 4 dòng\"}"},
      {"{\"text\":\"12345678901234567\"}", "{\"error\":\"mỗi dòng tối đa 16 ký tự\"}"},
      {"{\"text\":\"Xin ch\\u00f1o\"}", "{\"error\":\"text chỉ nhận ASCII in được và chữ cái tiếng Việt (UTF-8 dựng sẵn)\"}"},
      {"{}", "{\"error\":\"text phải là chuỗi\"}"},
  };
  for (const auto& bad : badCommands) {
    r.transport.simulateMessage(DISPLAY_CMD, bad.command);
    const auto* error = r.transport.last("domisen/entrance/display/error");
    TEST_ASSERT_NOT_NULL(error);
    TEST_ASSERT_FALSE(error->retain);
    TEST_ASSERT_EQUAL_STRING(bad.error, error->payload.c_str());
  }
  TEST_ASSERT_EQUAL(oldHistory, r.screen.history.size());
  TEST_ASSERT_EQUAL_STRING(oldState.c_str(), r.display.stateJson().c_str());
  TEST_ASSERT_EQUAL(oldLines.size(), r.screen.last.size());
  TEST_ASSERT_EQUAL_STRING("Xin chao", r.screen.last[0].c_str());
}

void test_oled_hien_thi_tieng_viet_co_dau_va_gui_lai_dung_state() {
  DisplayRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(DISPLAY_CMD, "{\"text\":\"Đèn phòng khách\\nBật\"}");
  TEST_ASSERT_EQUAL(2, r.screen.last.size());
  TEST_ASSERT_EQUAL_STRING("Đèn phòng khách", r.screen.last[0].c_str());
  TEST_ASSERT_EQUAL_STRING("Bật", r.screen.last[1].c_str());
  TEST_ASSERT_EQUAL_STRING("{\"text\":\"Đèn phòng khách\\nBật\"}", r.transport.last(DISPLAY_STATE)->payload.c_str());
  // Chữ có dấu viết bằng \u trong JSON cũng được nhận và hiện đúng.
  r.transport.simulateMessage(DISPLAY_CMD, "{\"text\":\"Xin ch\\u00e0o\"}");
  TEST_ASSERT_EQUAL_STRING("Xin chào", r.screen.last[0].c_str());
}

void test_oled_noi_lai_gui_lai_state_hien_tai() {
  DisplayRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(DISPLAY_CMD, "{\"text\":\"Dong 1\\nDong 2\"}");
  r.transport.simulateDisconnect();
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL_STRING("{\"text\":\"Dong 1\\nDong 2\"}",
                           r.transport.last(DISPLAY_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(DISPLAY_STATE)->retain);
}

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

// Cảm biến mưa dùng chống nhiễu 1 giây (docs/mqtt-topics.md): DO nhấp nháy khi nhỏ nước thì không báo.
void test_cam_bien_nhi_phan_chong_nhieu_1_giay_cho_cam_bien_mua() {
  FakeDigitalInput input;
  input.value = true;  // HIGH: khô (activeLow nên là chưa mưa)
  BinarySensorDevice rain("balcony/rain", "raining", input, true, 1000);
  TEST_ASSERT_TRUE(rain.tick(0));  // lần đọc đầu nhận ngay
  TEST_ASSERT_EQUAL_STRING("{\"raining\":false}", rain.stateJson().c_str());

  input.value = false;  // LOW: ướt
  TEST_ASSERT_FALSE(rain.tick(10));   // bắt đầu khoảng ổn định
  input.value = true;                 // nhấp nháy trở lại khô trước khi đủ 1 giây
  TEST_ASSERT_FALSE(rain.tick(500));
  input.value = false;
  TEST_ASSERT_FALSE(rain.tick(600));  // đếm lại từ 600
  TEST_ASSERT_FALSE(rain.tick(1599));
  TEST_ASSERT_EQUAL_STRING("{\"raining\":false}", rain.stateJson().c_str());
  TEST_ASSERT_TRUE(rain.tick(1600));  // đủ 1000 ms
  TEST_ASSERT_EQUAL_STRING("{\"raining\":true}", rain.stateJson().c_str());
}

// Công tắc từ MC-38 (docs/mqtt-topics.md): HIGH là cửa sổ mở (nam châm xa hoặc dây rút ra), LOW là đóng.
static const char* WINDOW_STATE = "domisen/bedroom/window/state";

void test_cong_tac_tu_cua_so_high_la_mo_low_la_dong() {
  FakeTransport transport;
  FakeClock clock;
  FakeDigitalInput input;
  BinarySensorDevice window("bedroom/window", "open", input, false);
  Hub hub{"guard", transport, clock, 5000};
  hub.add(window);
  hub.begin();
  TEST_ASSERT_EQUAL_STRING("{\"open\":null}", window.stateJson().c_str());  // trước lần đọc đầu
  input.value = true;                                                       // nam châm xa: mở
  transport.simulateConnect();
  hub.loop();
  TEST_ASSERT_EQUAL_STRING("{\"open\":true}", transport.last(WINDOW_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(transport.last(WINDOW_STATE)->retain);
  input.value = false;  // nam châm sát: đóng
  hub.loop();
  clock.advance(49);
  hub.loop();
  TEST_ASSERT_EQUAL_STRING("{\"open\":true}", transport.last(WINDOW_STATE)->payload.c_str());  // chưa đủ 50 ms
  clock.advance(1);
  hub.loop();
  TEST_ASSERT_EQUAL_STRING("{\"open\":false}", transport.last(WINDOW_STATE)->payload.c_str());
}

// ---- chạm nút cảm ứng bật tắt đèn trần (docs/mqtt-topics.md, mục "Nút cảm ứng điều khiển đèn trần") ----
static const char* WALL_BUTTON_STATE = "domisen/living/wall-button/state";

struct TouchRig {
  FakeTransport transport;
  FakeClock clock;
  FakeDigitalInput input;
  FakePwm pwm;
  DimmableLight ceiling{"living/ceiling-light", pwm};
  BinarySensorDevice wallButton{"living/wall-button", "pressed", input, false};
  TouchToggle toggle{wallButton, ceiling};
  Hub hub{"comfort", transport, clock, 5000};

  explicit TouchRig(bool firstReadWhileTouched = false) {
    hub.add(ceiling);
    hub.add(wallButton);
    hub.begin();
    toggle.begin();
    input.value = firstReadWhileTouched;
    hub.loop();  // lần đọc đầu của nút
  }
  void setTouch(bool touched) {
    input.value = touched;
    hub.loop();
    clock.advance(50);  // đủ thời gian chống nhiễu
    hub.loop();
    hub.loop();  // vòng sau mới gửi state của đèn (đèn được tick trước nút)
  }
  void hold(uint32_t ms) {
    for (uint32_t t = 0; t < ms; t += 100) {
      clock.advance(100);
      hub.loop();
    }
  }
};

void test_cham_bat_den_dang_tat_va_gui_state() {
  TouchRig r;
  r.transport.simulateConnect();
  r.setTouch(true);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"level\":100}", r.transport.last(CEILING_STATE)->payload.c_str());
  TEST_ASSERT_TRUE(r.transport.last(CEILING_STATE)->retain);
  TEST_ASSERT_EQUAL_STRING("{\"pressed\":true}", r.transport.last(WALL_BUTTON_STATE)->payload.c_str());  // state của nút không đổi
}

void test_cham_tat_den_dang_bat_giu_muc_cu_va_bat_lai_dung_muc() {
  TouchRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":true,\"level\":40}");
  r.setTouch(true);  // đang bật: tắt
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":40}", r.transport.last(CEILING_STATE)->payload.c_str());
  r.setTouch(false);  // nhả tay: không đổi
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":40}", r.transport.last(CEILING_STATE)->payload.c_str());
  r.setTouch(true);  // đang tắt: bật lại đúng mức 40 (độ sáng thật theo gamma 2,2: 0,4^2,2 = 0,1332)
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"level\":40}", r.transport.last(CEILING_STATE)->payload.c_str());
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1332f, r.pwm.last);
}

void test_cham_giu_tay_khong_lap_lai() {
  TouchRig r;
  r.transport.simulateConnect();
  r.setTouch(true);
  const int sent = r.transport.count(CEILING_STATE);
  r.hold(5000);
  TEST_ASSERT_EQUAL(sent, r.transport.count(CEILING_STATE));
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
}

void test_cham_nhieu_ngan_duoi_chong_nhieu_khong_doi_den() {
  TouchRig r;
  r.transport.simulateConnect();
  r.input.value = true;
  r.hub.loop();
  r.clock.advance(49);  // chưa đủ 50 ms
  r.input.value = false;
  r.hub.loop();
  r.clock.advance(100);
  r.hub.loop();
  r.hub.loop();
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":100}", r.ceiling.stateJson().c_str());
}

void test_cham_lan_doc_dau_dang_cham_khong_doi_den() {
  TouchRig r(true);  // tay đang đặt trên nút lúc cấp điện
  r.transport.simulateConnect();
  r.hold(1000);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":100}", r.ceiling.stateJson().c_str());
  r.setTouch(false);  // nhả tay: vẫn không đổi
  r.setTouch(true);   // chạm lần sau mới là một lần chạm
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
}

void test_cham_mat_ket_noi_van_doi_den_roi_noi_lai_gui_state_mot_lan() {
  TouchRig r;  // chưa nối broker
  r.setTouch(true);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
  TEST_ASSERT_EQUAL(0, r.transport.count(CEILING_STATE));
  r.transport.simulateConnect();
  TEST_ASSERT_EQUAL(1, r.transport.count(CEILING_STATE));
  TEST_ASSERT_EQUAL_STRING("{\"on\":true,\"level\":100}", r.transport.last(CEILING_STATE)->payload.c_str());
  r.hub.loop();
  r.hub.loop();
  TEST_ASSERT_EQUAL(1, r.transport.count(CEILING_STATE));  // không gửi lặp
}

void test_cham_xen_ke_lenh_mqtt_state_luon_dung_voi_den_that() {
  TouchRig r;
  r.transport.simulateConnect();
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":true}");
  r.setTouch(true);   // chạm: tắt
  r.setTouch(false);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":100}", r.transport.last(CEILING_STATE)->payload.c_str());
  r.transport.simulateMessage(CEILING_CMD, "{\"on\":true}");  // lệnh đến sau: bật
  TEST_ASSERT_EQUAL_FLOAT(1.0f, r.pwm.last);
  r.setTouch(true);  // chạm: tắt
  TEST_ASSERT_EQUAL_FLOAT(0.0f, r.pwm.last);
  TEST_ASSERT_EQUAL_STRING("{\"on\":false,\"level\":100}", r.transport.last(CEILING_STATE)->payload.c_str());
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
  RUN_TEST(test_gas_dang_lam_nong_state_null_va_chua_doc);
  RUN_TEST(test_gas_het_lam_nong_doc_8_mau_va_gui_ngay);
  RUN_TEST(test_gas_ppm_raw_la_trung_binh_8_mau);
  RUN_TEST(test_gas_alarm_bat_dung_nguong_va_gui_ngay);
  RUN_TEST(test_gas_alarm_co_tre_khong_nhap_nhay_quanh_nguong);
  RUN_TEST(test_gas_lay_mau_moi_giay_va_gui_moi_5_giay_khi_alarm_khong_doi);
  RUN_TEST(test_gas_chi_doc_khong_nhan_lenh);
  RUN_TEST(test_gas_noi_lai_gui_lai_state_hien_tai);
  RUN_TEST(test_gas_nguong_tat_cao_hon_nguong_bat_bi_ep_bang_nguong_bat);
  RUN_TEST(test_switch_setOn_tu_board_doi_output_va_bao_hub_gui_state_mot_lan);
  RUN_TEST(test_switch_khoa_tu_choi_lenh_tat_nhung_nhan_lenh_bat_va_mo_khoa_thi_tat_duoc);
  RUN_TEST(test_gas_an_toan_co_gas_bat_con_va_quat_hut_va_gui_state);
  RUN_TEST(test_gas_an_toan_het_gas_tat_ca_hai_va_bo_khoa);
  RUN_TEST(test_gas_an_toan_dang_bao_dong_tu_choi_lenh_tat_quat_hut);
  RUN_TEST(test_gas_an_toan_tat_tieng_coi_khong_tu_bat_lai_cho_toi_lan_bao_dong_sau);
  RUN_TEST(test_gas_an_toan_mat_ket_noi_van_bat_con_va_quat_hut_roi_gui_state_khi_noi_lai);
  RUN_TEST(test_gas_an_toan_chi_thiet_bi_an_toan_chay_khi_mat_ket_noi);
  RUN_TEST(test_anh_sang_chua_doc_state_null);
  RUN_TEST(test_anh_sang_doc_ngay_khi_noi_8_mau_va_gui_state);
  RUN_TEST(test_anh_sang_light_raw_la_trung_binh_8_mau);
  RUN_TEST(test_anh_sang_chuan_hoa_chieu_khi_ao_giam_luc_sang);
  RUN_TEST(test_anh_sang_dark_bat_dung_nguong_va_gui_ngay);
  RUN_TEST(test_anh_sang_dark_co_tre_khong_nhap_nhay_quanh_nguong);
  RUN_TEST(test_anh_sang_dark_tat_khi_dat_nguong_sang_roi_khong_tu_bat_o_giua);
  RUN_TEST(test_anh_sang_lay_mau_moi_giay_va_gui_moi_10_giay_khi_dark_khong_doi);
  RUN_TEST(test_anh_sang_chi_doc_khong_nhan_lenh);
  RUN_TEST(test_anh_sang_noi_lai_gui_lai_state_hien_tai);
  RUN_TEST(test_anh_sang_mat_ket_noi_thi_khong_lay_mau);
  RUN_TEST(test_anh_sang_nguong_sang_thap_hon_nguong_toi_bi_ep_bang_nguong_toi);
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
  RUN_TEST(test_oled_khoi_dong_trong_va_gui_state_retain);
  RUN_TEST(test_oled_hien_thi_mot_dong_va_bon_dong_moi_dong_16_ky_tu);
  RUN_TEST(test_oled_tu_choi_lenh_sai_va_giu_nguyen_man_hinh);
  RUN_TEST(test_oled_hien_thi_tieng_viet_co_dau_va_gui_lai_dung_state);
  RUN_TEST(test_oled_noi_lai_gui_lai_state_hien_tai);

  RUN_TEST(test_cam_bien_nhi_phan_chua_doc_gui_null_va_lan_dau_nhan_ngay);
  RUN_TEST(test_cam_bien_nhi_phan_chong_nhieu_va_nhan_gia_tri_on_dinh_sau_50ms);
  RUN_TEST(test_cam_bien_nhi_phan_bo_qua_nhieu_ngan_hon_50ms);
  RUN_TEST(test_cam_bien_nhi_phan_active_low_dao_muc);
  RUN_TEST(test_cam_bien_nhi_phan_chong_nhieu_1_giay_cho_cam_bien_mua);
  RUN_TEST(test_cong_tac_tu_cua_so_high_la_mo_low_la_dong);
  RUN_TEST(test_cham_bat_den_dang_tat_va_gui_state);
  RUN_TEST(test_cham_tat_den_dang_bat_giu_muc_cu_va_bat_lai_dung_muc);
  RUN_TEST(test_cham_giu_tay_khong_lap_lai);
  RUN_TEST(test_cham_nhieu_ngan_duoi_chong_nhieu_khong_doi_den);
  RUN_TEST(test_cham_lan_doc_dau_dang_cham_khong_doi_den);
  RUN_TEST(test_cham_mat_ket_noi_van_doi_den_roi_noi_lai_gui_state_mot_lan);
  RUN_TEST(test_cham_xen_ke_lenh_mqtt_state_luon_dung_voi_den_that);
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
