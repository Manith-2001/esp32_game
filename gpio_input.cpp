#include "gpio_input.h"

#include "Arduino.h"
#include "button_pins.h"

GpioInput::GpioInput()
    : jump_was_pressed(false), jump_polls_remaining(0) {
    // The buttons use external pull-down resistors and are active HIGH.
    pinMode(ButtonPins::LEFT, INPUT);
    pinMode(ButtonPins::RIGHT, INPUT);
    pinMode(ButtonPins::JUMP, INPUT);

    // Do not treat a button held during startup as a new jump press.
    jump_was_pressed = digitalRead(ButtonPins::JUMP) == HIGH;
}

InputState GpioInput::poll() {
    const bool left = digitalRead(ButtonPins::LEFT) == HIGH;
    const bool right_pressed = digitalRead(ButtonPins::RIGHT) == HIGH;
    // If the wiring makes both lines HIGH, treat the input as left only.
    const bool right = right_pressed && !left;
    const bool jump_pressed = digitalRead(ButtonPins::JUMP) == HIGH;

    if (jump_pressed && !jump_was_pressed)
        jump_polls_remaining = JUMP_LATCH_POLLS;

    // Keep the one-shot event visible while the slower sim task catches up.
    const bool jump = jump_polls_remaining > 0;
    if (jump_polls_remaining > 0)
        --jump_polls_remaining;

    jump_was_pressed = jump_pressed;
    return {left, right, jump};
}
