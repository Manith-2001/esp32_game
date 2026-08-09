#include "gpio_input.h"

#include "Arduino.h"
#include "button_pins.h"

GpioInput::GpioInput() : jump_was_pressed(false) {
    // The buttons use external pull-down resistors and are active HIGH.
    pinMode(ButtonPins::LEFT, INPUT);
    pinMode(ButtonPins::RIGHT, INPUT);
    pinMode(ButtonPins::JUMP, INPUT);

    // Do not treat a button held during startup as a new jump press.
    jump_was_pressed = digitalRead(ButtonPins::JUMP) == HIGH;
}

InputState GpioInput::poll() {
    const bool left = digitalRead(ButtonPins::LEFT) == HIGH;
    const bool right = digitalRead(ButtonPins::RIGHT) == HIGH;
    const bool jump_pressed = digitalRead(ButtonPins::JUMP) == HIGH;
    const bool jump = jump_pressed && !jump_was_pressed;

    jump_was_pressed = jump_pressed;
    return {left, right, jump};
}
