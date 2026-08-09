#pragma once

#include "input.h"

class GpioInput : public Input {
public:
    GpioInput();

    InputState poll() override;

private:
    bool jump_was_pressed;
};
