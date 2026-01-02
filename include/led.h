#ifndef LED_H
#define LED_H

#include <Arduino.h>

#define MAIN_RED_LED_PIN 5
#define MAIN_YELLOW_LED_PIN 4
#define MAIN_GREEN_LED_PIN 0
#define RED_LED_PIN 14
#define GREEN_LED_PIN 12

void led_pin_init(void);
void led_set_state(int ledPin, bool state);
void set_main_led(bool red, bool yellow, bool green);
void set_ped_led(bool red, bool green);
void set_safeMode_led();

#endif