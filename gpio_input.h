#pragma once

#include <cstdint>

#include "input.h"

class GpioInput : public Input {
public:
    GpioInput();

    InputState poll() override;

private:
    static constexpr uint8_t JUMP_LATCH_POLLS = 6;

    bool jump_was_pressed;
    uint8_t jump_polls_remaining;
};
