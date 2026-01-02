#include "unity.h"
#include "led.h"
#include <cstring>

#ifndef HIGH
#define HIGH 1
#define LOW 0
#endif

#ifndef OUTPUT
#define OUTPUT 1
#endif

// Makieta backendu GPIO
static const int MAX_PINS = 64;
static int mock_pin_mode[MAX_PINS];
static int mock_pin_state[MAX_PINS];

static int get_pin_state(int pin) {
    if (pin < 0 || pin >= MAX_PINS) return -1;
    return mock_pin_state[pin];
}

// Fałszywa implementacja używana przez led.cpp
void pinMode(int pin, int mode) {
    if (pin < 0 || pin >= MAX_PINS) return;
    mock_pin_mode[pin] = mode;
}

void digitalWrite(int pin, int val) {
    if (pin < 0 || pin >= MAX_PINS) return;
    mock_pin_state[pin] = val;
}

// Unity hooks
void setUp(void) {
    std::memset(mock_pin_mode, 0, sizeof(mock_pin_mode));
    // wstępnie ustaw stany na HIGH, aby upewnić się, że set_safeMode_led je wyzeruje
    for (int i = 0; i < MAX_PINS; ++i) mock_pin_state[i] = HIGH;
    // Zainicjalizuj piny tak jak w produkcji:
    led_pin_init();
}

void tearDown(void) {
    // brak operacji
}

// Test: set_safeMode_led powinien ustawić wszystkie istotne piny na LOW
void test_set_safeMode_led_sets_all_outputs_low(void) {
    // Upewnij się, że początkowy stan jest HIGH
    TEST_ASSERT_EQUAL_INT(HIGH, get_pin_state(MAIN_RED_LED_PIN));
    TEST_ASSERT_EQUAL_INT(HIGH, get_pin_state(MAIN_YELLOW_LED_PIN));
    TEST_ASSERT_EQUAL_INT(HIGH, get_pin_state(MAIN_GREEN_LED_PIN));
    TEST_ASSERT_EQUAL_INT(HIGH, get_pin_state(RED_LED_PIN));
    TEST_ASSERT_EQUAL_INT(HIGH, get_pin_state(GREEN_LED_PIN));

    // Wywołaj testowaną funkcję
    set_safeMode_led();

    // Sprawdź, że wszystkie są LOW
    TEST_ASSERT_EQUAL_INT(LOW, get_pin_state(MAIN_RED_LED_PIN));
    TEST_ASSERT_EQUAL_INT(LOW, get_pin_state(MAIN_YELLOW_LED_PIN));
    TEST_ASSERT_EQUAL_INT(LOW, get_pin_state(MAIN_GREEN_LED_PIN));
    TEST_ASSERT_EQUAL_INT(LOW, get_pin_state(RED_LED_PIN));
    TEST_ASSERT_EQUAL_INT(LOW, get_pin_state(GREEN_LED_PIN));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_set_safeMode_led_sets_all_outputs_low);
    return UNITY_END();
}