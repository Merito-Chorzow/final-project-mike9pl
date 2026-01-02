#include <Arduino.h>
#include "button.h"
#include "led.h"
#include "task.h"
#include "uart.h"

void setup()
{
    uart_init(115200);
    button_pin_init();
    led_pin_init();

    if (!task_begin()) 
    {
        uart_log_line("SYS: FAILED to start tasks\n\r");
    }else 
    {
        uart_log_line("SYS: Tasks started\n\r");
    }

    uart_log_line("SYSTEM READY\n\r");
}

void loop()
{
    // pętla pusta - logika obsługiwana przez FreeRTOS
}
