#ifndef TASK_H
#define TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Inicjuje i uruchamia zadania FreeRTOS oraz kolejkę zdarzeń. Zwraca false w przypadku niepowodzenia.
bool task_begin();

// Punkty wejścia zadań (FreeRTOS)
void fsmTask(void *pvParameters);
void buttonTask(void *pvParameters);

#endif
