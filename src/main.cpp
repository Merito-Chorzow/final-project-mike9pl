#include <Arduino.h>
#include "button.h"
#include "led.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

static QueueHandle_t buttonQueue = NULL;
static TaskHandle_t safeTaskHandle = NULL;
static volatile bool safeActive = false;

static void buttonTask(void *pvParameters)
{
    (void)pvParameters;
    uint8_t lastRead = digitalRead(BUTTON_PIN);
    uint8_t stableState = lastRead;
    unsigned long lastChangeMs = millis();
    const TickType_t sampleDelay = pdMS_TO_TICKS(10);
    const unsigned long debounceMs = 50;
    const unsigned long SAFE_ENTER_MS = 3000; // long press to enter SAFE
    const unsigned long SAFE_EXIT_MS  = 3000; // long press to exit SAFE
    unsigned long pressStartMs = 0;

    while (true)
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
                // debounced change
                stableState = lastRead;
                Serial.print("Button debounced: ");
                Serial.println(stableState == LOW ? "PRESSED" : "RELEASED");

                if (buttonQueue) {
                    xQueueSend(buttonQueue, &stableState, 0);
                    Serial.print("Enqueued button event: ");
                    Serial.println(stableState == LOW ? "PRESSED" : "RELEASED");
                }

                // long-press detection: start on press, evaluate on release
                if (stableState == LOW) {
                    pressStartMs = millis();
                    Serial.println("Button press start recorded");
                } else { // released
                    if (pressStartMs != 0) {
                        unsigned long pressDur = millis() - pressStartMs;
                        pressStartMs = 0;
                        Serial.print("Button released. Duration ms: ");
                        Serial.println(pressDur);

                        // If SAFE mode active: require 3s press to exit
                        if (safeActive && pressDur >= SAFE_EXIT_MS && safeTaskHandle) {
                            Serial.println("SAFE MODE: exit long-press detected (>=3s) - notifying SAFE task to exit");
                            xTaskNotifyGive(safeTaskHandle);
                        }
                        // If not in SAFE mode: allow enter with configured shorter long press
                        else if (!safeActive && pressDur >= SAFE_ENTER_MS && safeTaskHandle) {
                            Serial.println("Long press detected - notifying SAFE task to enter");
                            xTaskNotifyGive(safeTaskHandle);
                        }
                    }
                }
            }
        }
        vTaskDelay(sampleDelay);
    }
}

static void safeTask(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        // wait until notified to enter SAFE mode
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // toggle safe mode on
        safeActive = true;
        Serial.println("SAFE MODE: ENTER");

        // ensure all non-yellow leds off initially
        Serial.println("SAFE MODE: turning off non-yellow LEDs");
        led_set_state(MAIN_RED_LED_PIN, LOW);
        led_set_state(MAIN_GREEN_LED_PIN, LOW);
        led_set_state(RED_LED_PIN, LOW);
        led_set_state(GREEN_LED_PIN, LOW);

        // blink main yellow until another notify toggles off
        bool running = true;
        while (running)
        {
            led_set_state(MAIN_YELLOW_LED_PIN, HIGH);
            Serial.println("SAFE MODE: MAIN_YELLOW ON");
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500))) { running = false; break; }
            led_set_state(MAIN_YELLOW_LED_PIN, LOW);
            Serial.println("SAFE MODE: MAIN_YELLOW OFF");
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500))) { running = false; break; }
        }

        // exit safe mode - perform exit action (indicator blink) and restore MAIN_RED
        led_set_state(MAIN_YELLOW_LED_PIN, LOW);
        Serial.println("SAFE MODE: EXIT - performing exit action");

        // small indicator: blink MAIN_RED twice, then leave MAIN_RED ON
        for (int i = 0; i < 2; ++i) {
            led_set_state(MAIN_RED_LED_PIN, HIGH);
            Serial.println("SAFE MODE: EXIT - MAIN_RED BLINK ON");
            vTaskDelay(pdMS_TO_TICKS(200));
            led_set_state(MAIN_RED_LED_PIN, LOW);
            Serial.println("SAFE MODE: EXIT - MAIN_RED BLINK OFF");
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        // restore main red as default resume state
        led_set_state(MAIN_RED_LED_PIN, HIGH);
        Serial.println("SAFE MODE: EXIT - MAIN_RED RESTORED");

        safeActive = false;
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
    Serial.println("LED TASK: initialized MAIN_RED ON");

    while (true)
    {
        // if safe mode active, skip normal cycle
        if (safeActive) {
            vTaskDelay(checkPeriod);
            continue;
        }

        // check for button events (non-blocking)
        uint8_t btnState;
        if (xQueueReceive(buttonQueue, &btnState, 0) == pdTRUE)
        {
            if (btnState == LOW) // pressed -> override sequence
            {
                Serial.println("LED TASK: Button override sequence START");

                // ensure only main RED is on
                led_set_state(MAIN_RED_LED_PIN, HIGH);
                led_set_state(MAIN_YELLOW_LED_PIN, LOW);
                led_set_state(MAIN_GREEN_LED_PIN, LOW);
                Serial.println("LED TASK: MAIN_RED ON, MAIN_YELLOW OFF, MAIN_GREEN OFF");

                // secondary leds: RED -> 3s -> GREEN -> 3s -> RED (end)
                led_set_state(RED_LED_PIN, HIGH);
                led_set_state(GREEN_LED_PIN, LOW);
                Serial.println("LED TASK: SECONDARY RED ON");
                vTaskDelay(dur_red);

                led_set_state(RED_LED_PIN, LOW);
                led_set_state(GREEN_LED_PIN, HIGH);
                Serial.println("LED TASK: SECONDARY GREEN ON");
                vTaskDelay(dur_green);

                // end with secondary RED on and main RED on
                led_set_state(RED_LED_PIN, HIGH);
                led_set_state(GREEN_LED_PIN, LOW);
                Serial.println("LED TASK: SECONDARY RED restored, override END");

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
                    Serial.println("MAIN STATE -> YELLOW (1s)");
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
                    Serial.println("MAIN STATE -> GREEN (3s)");
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
                    Serial.println("MAIN STATE -> YELLOW (1s)");
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
                    Serial.println("MAIN STATE -> RED (3s)");
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
    Serial.println("Task created: ButtonTask");
    xTaskCreatePinnedToCore(ledTask,    "LedTask",    2048, NULL, 1, NULL, 1);
    Serial.println("Task created: LedTask");
    xTaskCreatePinnedToCore(safeTask,   "SafeTask",   2048, NULL, 3, &safeTaskHandle, 1);
    Serial.println("Task created: SafeTask");

    Serial.println("SYSTEM READY!!!");
}

void loop()
{
    // empty - logic is handled in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}
