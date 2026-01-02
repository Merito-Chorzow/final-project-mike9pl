#include "led.h"

void led_pin_init() {
    // Inicjalizuje sprzęt LED (konfiguracja pinu GPIO)
    pinMode(MAIN_RED_LED_PIN, OUTPUT);
    pinMode(MAIN_YELLOW_LED_PIN, OUTPUT);
    pinMode(MAIN_GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);

    // Bezpieczne ustawienia domyślne: główne czerwone oraz czerwone dla pieszych włączone.
    digitalWrite(MAIN_RED_LED_PIN, HIGH);
    digitalWrite(MAIN_YELLOW_LED_PIN, LOW);
    digitalWrite(MAIN_GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
}

void led_set_state(int ledPin, bool state) {
    // Ustaw stan LED (włącz/wyłącz)
    digitalWrite(ledPin, state ? HIGH : LOW);
}
// Ustawienia stanów LED dla głównych świateł
void set_main_led(bool red, bool yellow, bool green)
{
    led_set_state(MAIN_RED_LED_PIN, red);
    led_set_state(MAIN_YELLOW_LED_PIN, yellow);
    led_set_state(MAIN_GREEN_LED_PIN, green);
}
// Ustawienia stanów LED dla świateł pieszych
void set_ped_led(bool red, bool green)
{
    led_set_state(RED_LED_PIN, red);
    led_set_state(GREEN_LED_PIN, green);
}
// Ustawienia stanów LED dla trybu SAFE
void set_safeMode_led()
{
    // tylko GŁÓWNY ŻÓŁTY będzie używany do migania; pozostałe wyjścia wyłączone.
    set_main_led(false, false, false);
    set_ped_led(false, false);
}