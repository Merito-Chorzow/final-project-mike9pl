#include "task.h"
#include "fsm.h"
#include "button.h"
#include "uart.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

static QueueHandle_t s_eventQueue = NULL;
// Inicjuje i uruchamia zadania FreeRTOS oraz kolejkę zdarzeń. Zwraca false w przypadku niepowodzenia.
bool task_begin()
{
    s_eventQueue = xQueueCreate(10, sizeof(ButtonEvent));
    if (s_eventQueue == NULL) {
        uart_log_line("TASK: FAILED to create event queue\n\r");
        return false;
    }
    // rejestruj kolejkę
    set_button_queue(s_eventQueue);

    if (xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, NULL, 2, NULL, 1) != pdPASS) // wyższy priorytet dla przycisku
    {
        uart_log_line("TASK: FAILED to create ButtonTask\n\r");
        return false;
    }

    if (xTaskCreatePinnedToCore(fsmTask, "FsmTask", 4096, NULL, 1, NULL, 1) != pdPASS) 
    {
        uart_log_line("TASK: FAILED to create FsmTask\n\r");
        return false;
    }

    return true;
}

void fsmTask(void *pvParameters)
{
	(void)pvParameters;
    fsm_set_state(FsmState::INIT);  // start w stanie INIT
	fsm_init();                     // uruchom pętlę FSM
}

void buttonTask(void *pvParameters)
{
    (void)pvParameters;
    check_button_press(s_eventQueue); // uruchom pętlę obsługi przycisku
    
}
