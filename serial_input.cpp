#include "serial_input.h"
#include <Arduino.h>

InputState SerialInput::poll() {
    unsigned long now = millis();

    while (Serial.available() > 0) {
        char c = Serial.read();
        switch (c) {
            case 'a': left_key.active = true;  left_key.last_seen = now;  break;
            case 'd': right_key.active = true; right_key.last_seen = now; break;
            case 'w': jump_key.active = true;  jump_key.last_seen = now;  break;
        }
    }

    if (left_key.active  && (now - left_key.last_seen  > TIMEOUT_MS)) left_key.active  = false;
    if (right_key.active && (now - right_key.last_seen > TIMEOUT_MS)) right_key.active = false;
    if (jump_key.active  && (now - jump_key.last_seen > TIMEOUT_MS)) jump_key.active  = false;

    return {left_key.active, right_key.active, jump_key.active};
}
