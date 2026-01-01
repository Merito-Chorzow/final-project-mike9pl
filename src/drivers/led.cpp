#include "led.h"
void led_init() {
    // Initialize LED hardware (e.g., configure GPIO pin)
    pinMode(MAIN_RED_LED_PIN, OUTPUT);
    pinMode(MAIN_YELLOW_LED_PIN, OUTPUT);
    pinMode(MAIN_GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);

    // Safe defaults: cars RED on, pedestrians RED on.
    digitalWrite(MAIN_RED_LED_PIN, HIGH);
    digitalWrite(MAIN_YELLOW_LED_PIN, LOW);
    digitalWrite(MAIN_GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
}
void led_set_state(int ledPin, bool state) {
    // Set the LED state (on/off)
    digitalWrite(ledPin, state ? HIGH : LOW);
}