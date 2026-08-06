#include "player_animation.h"

#include "Arduino.h"
#include "player_run_sheet.h"

PlayerAnimation::PlayerAnimation()
    : frame_index(0), frame_tick(0), facing_left(false) {}

void PlayerAnimation::update(float vx, bool grounded, int direction) {
    if (direction < 0)
        facing_left = true;
    else if (direction > 0)
        facing_left = false;

    const bool moving = vx > RUN_THRESHOLD || vx < -RUN_THRESHOLD;
    if (!grounded || !moving) {
        resetPlayback();
        return;
    }

    frame_tick++;
    if (frame_tick < FRAME_TICKS)
        return;

    frame_tick = 0;
    frame_index++;
    if (frame_index >= FRAME_COUNT)
        frame_index = 0;
}

void PlayerAnimation::reset() {
    resetPlayback();
    facing_left = false;
}

void PlayerAnimation::resetPlayback() {
    frame_index = 0;
    frame_tick = 0;
}

const uint16_t *PlayerAnimation::pixels() const {
    return player_run_sheet + frame_index * FRAME_PIXELS;
}

bool PlayerAnimation::flipX() const {
    return facing_left;
}
