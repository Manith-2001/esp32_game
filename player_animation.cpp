#include "player_animation.h"

#include "Arduino.h"
#include "manith_run_frame_1.h"
#include "manith_run_frame_2.h"
#include "manith_sprite.h"

PlayerAnimation::PlayerAnimation()
    : frame_index(0), frame_tick(0), facing_left(false),
      state(State::GROUNDED) {}

void PlayerAnimation::update(float vx, float vy, bool grounded,
                             int direction) {
    if (direction < 0)
        facing_left = true;
    else if (direction > 0)
        facing_left = false;

    if (!grounded) {
        const State airborne_state =
            vy < 0.0f ? State::JUMP_RISING : State::JUMP_FALLING;

        if (state != airborne_state) {
            state = airborne_state;
            resetPlayback();
        }
        return;
    }

    if (state != State::GROUNDED) {
        state = State::GROUNDED;
        resetPlayback();
    }

    const bool moving = vx > RUN_THRESHOLD || vx < -RUN_THRESHOLD;
    if (!moving) {
        resetPlayback();
        return;
    }

    frame_tick++;
    if (frame_tick < RUN_FRAME_TICKS)
        return;

    frame_tick = 0;
    frame_index++;
    if (frame_index >= RUN_FRAME_COUNT)
        frame_index = 0;
}

void PlayerAnimation::reset() {
    resetPlayback();
    facing_left = false;
    state = State::GROUNDED;
}

void PlayerAnimation::resetPlayback() {
    frame_index = 0;
    frame_tick = 0;
}

const uint16_t *PlayerAnimation::pixels() const {
    if (state == State::JUMP_RISING || state == State::JUMP_FALLING)
        return manith_run_frame_1;

    switch (frame_index) {
    case 1:
        return manith_run_frame_1;
    case 3:
        return manith_run_frame_2;
    default:
        return manith_sprite;
    }
}

bool PlayerAnimation::flipX() const {
    // The source art faces left, so right-facing renders need the flip.
    return !facing_left;
}
