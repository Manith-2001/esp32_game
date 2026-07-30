#pragma once
#include "input.h"

class SerialInput : public Input {
public:
    InputState poll() override;

private:
    struct KeyState {
        bool active;
        unsigned long last_seen;
    };
    static constexpr unsigned long TIMEOUT_MS = 100;

    KeyState left_key  = {false, 0};
    KeyState right_key = {false, 0};
    KeyState jump_key  = {false, 0};
};
