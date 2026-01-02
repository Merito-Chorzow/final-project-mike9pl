#include <Arduino.h>
#include <unity.h>

void test_dummy_true() {
  TEST_ASSERT_TRUE(1);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_dummy_true);
  UNITY_END();
}

void loop() {}
