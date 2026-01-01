#include "button.h"


// Example GPIO pin for the button

void button_init() {
    // Initialize button hardware (e.g., configure GPIO pin)
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}
bool button_is_pressed() {
    // INPUT_PULLUP => pressed when pin reads LOW
    return digitalRead(BUTTON_PIN) == LOW;
}