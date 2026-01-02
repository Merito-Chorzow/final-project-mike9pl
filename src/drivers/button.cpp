#include "button.h"
#include "uart.h"

static QueueHandle_t buttonQueue = NULL;

void set_button_queue(QueueHandle_t q) {
    buttonQueue = q;
}

void button_pin_init() {
    // Inicjalizuja przycisku
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

int read_button()
{
    return digitalRead(BUTTON_PIN);
}
// Zwraca zarejestrowaną kolejkę zdarzeń przycisku
QueueHandle_t get_button_queue()
{
    return buttonQueue;
}
// Sprawdzanie stanu przycisku i wysyłanie zdarzenia do kolejki,
// wykrywanie krótkich i długich naciśnięć
void check_button_press(QueueHandle_t eventQueue)
{
    if (buttonQueue == NULL) {
        // zarejestruj podaną kolejkę, jeśli jeszcze nie ustawiono
        buttonQueue = eventQueue;
    }

    int lastRead = read_button();
    int stableState = lastRead;                         // stan po de-bounceniu
    unsigned long lastChangeMs = millis();              // czas ostatniej zmiany stanu
    const TickType_t sampleDelay = pdMS_TO_TICKS(10);   // 10 ms próbkowanie
    const unsigned long debounceMs = 50;                // 50 ms de-bounce
    const unsigned long LONG_PRESS_MS = 3000;           // 3 sekundy dla długiego naciśnięcia
    unsigned long pressStartMs = 0;                     // czas rozpoczęcia naciśnięcia

    while (true)
    {
        int cur = read_button();
        // logika de-bounce
        if (cur != lastRead)
        {
            lastChangeMs = millis();
            lastRead = cur;
        }
        else
        {
            if ((millis() - lastChangeMs) >= debounceMs && lastRead != stableState)
            {
                stableState = lastRead;
                uart_log_line(stableState == LOW ? "BTN: Press Start\n\r" : "BTN: Released\n\r");
                // wykrywanie naciśnięcia/zwolnienia
                if (stableState == LOW) {
                    pressStartMs = millis();
                } else {
                    if (pressStartMs != 0) {
                        unsigned long pressDur = millis() - pressStartMs;
                        pressStartMs = 0;
                        uart_log_line("BTN: Duration ms: ");
                        uart_log_int(pressDur);
                        uart_log_line("\n\r");
                        // wyślij zdarzenie do kolejki
                        if (buttonQueue != NULL) {
                            ButtonEvent ev;
                            // zdecyduj czy to krótkie czy długie naciśnięcie
                            if (pressDur >= LONG_PRESS_MS) {
                                ev = ButtonEvent{ButtonEventType::LongPress, static_cast<uint32_t>(pressDur)};
                                uart_log_line("BTN->FSM: LongPress\n\r");
                            } else {
                                ev = ButtonEvent{ButtonEventType::ShortPress, static_cast<uint32_t>(pressDur)};
                                uart_log_line("BTN->FSM: ShortPress\n\r");
                            }

                            xQueueSend(buttonQueue, &ev, 0);
                        }
                    }
                }
            }
        }
        vTaskDelay(sampleDelay);
    }
}