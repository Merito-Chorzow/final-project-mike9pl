#ifndef LED_H
#define LED_H

#include <Arduino.h>

#define MAIN_RED_LED_PIN 5
#define MAIN_YELLOW_LED_PIN 4
#define MAIN_GREEN_LED_PIN 0
#define RED_LED_PIN 14
#define GREEN_LED_PIN 12

void led_init(void);
void led_set_state(int ledPin, bool state);

#endif