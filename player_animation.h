#pragma once

#include <cstdint>

class PlayerAnimation {
public:
    PlayerAnimation();

    // Advance the current clip from the final player physics state.
    // direction is -1 for left, 1 for right, and 0 for no new input.
    void update(float vx, float vy, bool grounded, int direction);
    void reset();

    const uint16_t *pixels() const;
    bool flipX() const;

private:
    enum class State : uint8_t {
        GROUNDED,
        JUMP_RISING,
        JUMP_FALLING
    };

    static constexpr float RUN_THRESHOLD = 0.1f;
    static constexpr uint8_t RUN_FRAME_COUNT = 4;
    static constexpr uint8_t RUN_FRAME_TICKS = 4;

    void resetPlayback();

    uint8_t frame_index;
    uint8_t frame_tick;
    bool facing_left;
    State state;
};
