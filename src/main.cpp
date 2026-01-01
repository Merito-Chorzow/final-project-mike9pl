#include <Arduino.h>
#include "button.h"
#include "led.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

static QueueHandle_t buttonQueue = NULL;

static void buttonTask(void *pvParameters)
{
    (void)pvParameters;
    uint8_t lastRead = digitalRead(BUTTON_PIN);
    uint8_t stableState = lastRead;
    unsigned long lastChangeMs = millis();
    const TickType_t sampleDelay = pdMS_TO_TICKS(10);
    const unsigned long debounceMs = 50;

    for (;;)
    {
        uint8_t cur = digitalRead(BUTTON_PIN);
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
                if (buttonQueue) xQueueSend(buttonQueue, &stableState, 0);
            }
        }
        vTaskDelay(sampleDelay);
    }
}

static void ledTask(void *pvParameters)
{
    (void)pvParameters;

    enum MainState { MS_RED, MS_YELLOW1, MS_GREEN, MS_YELLOW2 };
    MainState current = MS_RED;

    const TickType_t checkPeriod = pdMS_TO_TICKS(50);
    const TickType_t dur_red    = pdMS_TO_TICKS(3000); // 3s
    const TickType_t dur_y1     = pdMS_TO_TICKS(1000); // 1s
    const TickType_t dur_green  = pdMS_TO_TICKS(3000); // 3s
    const TickType_t dur_y2     = pdMS_TO_TICKS(2000); // 2s

    TickType_t stateStart = xTaskGetTickCount();

    // initialize main leds to starting state
    led_set_state(MAIN_RED_LED_PIN, HIGH);
    led_set_state(MAIN_YELLOW_LED_PIN, LOW);
    led_set_state(MAIN_GREEN_LED_PIN, LOW);

    while (true)
    {
        // check for button events (non-blocking)
        uint8_t btnState;
        if (xQueueReceive(buttonQueue, &btnState, 0) == pdTRUE)
        {
            if (btnState == LOW) // pressed -> override sequence
            {
                // ensure only main RED is on
                led_set_state(MAIN_RED_LED_PIN, HIGH);
                led_set_state(MAIN_YELLOW_LED_PIN, LOW);
                led_set_state(MAIN_GREEN_LED_PIN, LOW);

                // secondary leds: RED -> 3s -> GREEN -> 3s -> RED (end)
                led_set_state(RED_LED_PIN, HIGH);
                led_set_state(GREEN_LED_PIN, LOW);

                vTaskDelay(dur_red);

                led_set_state(RED_LED_PIN, LOW);
                led_set_state(GREEN_LED_PIN, HIGH);

                vTaskDelay(dur_green);

                // end with secondary RED on and main RED on
                led_set_state(RED_LED_PIN, HIGH);
                led_set_state(GREEN_LED_PIN, LOW);

                current = MS_RED;
                stateStart = xTaskGetTickCount();
            }
            // ignore release events
        }

        // handle normal cycle timing
        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed = now - stateStart;

        switch (current)
        {
            case MS_RED:
                if (elapsed >= dur_red)
                {
                    current = MS_YELLOW1;
                    stateStart = now;
                    led_set_state(MAIN_RED_LED_PIN, LOW);
                    led_set_state(MAIN_YELLOW_LED_PIN, HIGH);
                    led_set_state(MAIN_GREEN_LED_PIN, LOW);
                }
                break;

            case MS_YELLOW1:
                if (elapsed >= dur_y1)
                {
                    current = MS_GREEN;
                    stateStart = now;
                    led_set_state(MAIN_RED_LED_PIN, LOW);
                    led_set_state(MAIN_YELLOW_LED_PIN, LOW);
                    led_set_state(MAIN_GREEN_LED_PIN, HIGH);
                }
                break;

            case MS_GREEN:
                if (elapsed >= dur_green)
                {
                    current = MS_YELLOW2;
                    stateStart = now;
                    led_set_state(MAIN_RED_LED_PIN, LOW);
                    led_set_state(MAIN_YELLOW_LED_PIN, HIGH);
                    led_set_state(MAIN_GREEN_LED_PIN, LOW);
                }
                break;

            case MS_YELLOW2:
                if (elapsed >= dur_y2)
                {
                    current = MS_RED;
                    stateStart = now;
                    led_set_state(MAIN_RED_LED_PIN, HIGH);
                    led_set_state(MAIN_YELLOW_LED_PIN, LOW);
                    led_set_state(MAIN_GREEN_LED_PIN, LOW);
                }
                break;
        }

        vTaskDelay(checkPeriod);
    }
}

int oldValue = HIGH; // default/idle value for pin 4 is high.

void setup()
{
    Serial.begin(115200);
    button_init();
    led_init();

    buttonQueue = xQueueCreate(10, sizeof(uint8_t));

    // Create FreeRTOS tasks (pin to core 1 for Arduino on ESP32)
    xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(ledTask,    "LedTask",    2048, NULL, 1, NULL, 1);

    Serial.println("SYSTEM READY!!!");
}

void loop()
{
    // empty - logic is handled in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}
