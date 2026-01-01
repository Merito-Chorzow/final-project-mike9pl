#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

#define BUTTON_PIN 2

void button_init(void);
bool button_is_pressed(void);

#endif