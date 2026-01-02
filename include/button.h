#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define BUTTON_PIN 2

void button_pin_init();
int read_button();
void check_button_press(QueueHandle_t eventQueue);
QueueHandle_t get_button_queue();
void set_button_queue(QueueHandle_t q);

// Zdarzenia przycisku
enum class ButtonEventType : uint8_t {
	ShortPress,
	LongPress,
};
// Struktura zdarzenia przycisku
struct ButtonEvent {
	ButtonEventType type;
	uint32_t durationMs;
};

#endif