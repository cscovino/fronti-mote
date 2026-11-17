#include <unity.h>

#include "link_state.h"

void setUp() {}
void tearDown() {}

static void test_connected_is_online_at_once() {
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Connecting, true, 0,
                                  kLinkDebounceMs) == LinkStatus::Online);
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Online, true, 1,
                                  kLinkDebounceMs) == LinkStatus::Online);
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Offline, true, 0,
                                  kLinkDebounceMs) == LinkStatus::Online);
}

static void test_connecting_becomes_offline_after_the_debounce() {
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Connecting, false,
                                  kLinkDebounceMs - 1,
                                  kLinkDebounceMs) == LinkStatus::Connecting);
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Connecting, false,
                                  kLinkDebounceMs,
                                  kLinkDebounceMs) == LinkStatus::Offline);
}

static void test_online_survives_a_short_drop() {
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Online, false,
                                  kLinkDebounceMs - 1,
                                  kLinkDebounceMs) == LinkStatus::Online);
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Online, false, kLinkDebounceMs,
                                  kLinkDebounceMs) == LinkStatus::Offline);
}

static void test_offline_waits_for_a_connection() {
  TEST_ASSERT_TRUE(nextLinkStatus(LinkStatus::Offline, false, 60000,
                                  kLinkDebounceMs) == LinkStatus::Offline);
}

static void test_retry_waits_for_the_previous_attempt() {
  TEST_ASSERT_FALSE(
      wifiRetryDue(LinkStatus::Offline, false, 60000, 0, kWifiRetryIntervalMs));
}

static void test_retry_waits_for_the_gap() {
  TEST_ASSERT_FALSE(
      wifiRetryDue(LinkStatus::Offline, true, 9999, 0, kWifiRetryIntervalMs));
  TEST_ASSERT_TRUE(
      wifiRetryDue(LinkStatus::Offline, true, 10000, 0, kWifiRetryIntervalMs));
}

static void test_online_never_retries() {
  TEST_ASSERT_FALSE(
      wifiRetryDue(LinkStatus::Online, true, 60000, 0, kWifiRetryIntervalMs));
}

static void test_retry_survives_a_millis_rollover() {
  TEST_ASSERT_TRUE(wifiRetryDue(LinkStatus::Offline, true, 6000, 0xFFFFF000u,
                                kWifiRetryIntervalMs));
}

static void test_online_led_is_solid() {
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Online, 0));
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Online, 123456));
}

static void test_connecting_blinks_fast() {
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Connecting, 0));
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Connecting, 199));
  TEST_ASSERT_FALSE(ledOnForStatus(LinkStatus::Connecting, 200));
  TEST_ASSERT_FALSE(ledOnForStatus(LinkStatus::Connecting, 399));
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Connecting, 400));
}

static void test_offline_blinks_slow() {
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Offline, 0));
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Offline, 999));
  TEST_ASSERT_FALSE(ledOnForStatus(LinkStatus::Offline, 1000));
  TEST_ASSERT_TRUE(ledOnForStatus(LinkStatus::Offline, 2000));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_connected_is_online_at_once);
  RUN_TEST(test_connecting_becomes_offline_after_the_debounce);
  RUN_TEST(test_online_survives_a_short_drop);
  RUN_TEST(test_offline_waits_for_a_connection);
  RUN_TEST(test_retry_waits_for_the_previous_attempt);
  RUN_TEST(test_retry_waits_for_the_gap);
  RUN_TEST(test_online_never_retries);
  RUN_TEST(test_retry_survives_a_millis_rollover);
  RUN_TEST(test_online_led_is_solid);
  RUN_TEST(test_connecting_blinks_fast);
  RUN_TEST(test_offline_blinks_slow);
  return UNITY_END();
}
