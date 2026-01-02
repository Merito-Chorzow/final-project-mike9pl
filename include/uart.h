#ifndef UART_H
#define UART_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Prosty wrapper UART/Serial z mutexem zapobiegającym mieszaniu wyjścia.
void uart_init(unsigned long baud);

// Odczyt nieblokujący. Zwraca true jeśli odczytano bajt.
bool uart_read_byte(uint8_t &out);

// Zapis do UART.
void uart_write_bytes(const uint8_t *data, size_t len);

// Zapis logów do tego samego UART).
void uart_log_line(const char *s);
// Zapis logowania linii dla unsigned long
void uart_log_int(unsigned long v);
#endif
