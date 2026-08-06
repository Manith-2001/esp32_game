#pragma once

#include <cstdint>

class PlayerAnimation {
public:
    PlayerAnimation();

    // Advance the run clip from the final player physics state.
    // direction is -1 for left, 1 for right, and 0 for no new input.
    void update(float vx, bool grounded, int direction);
    void reset();

    const uint16_t *pixels() const;
    bool flipX() const;

private:
    static constexpr float RUN_THRESHOLD = 0.1f;
    static constexpr uint8_t FRAME_COUNT = 4;
    static constexpr uint8_t FRAME_TICKS = 4;
    static constexpr uint16_t FRAME_PIXELS = 24 * 24;

    void resetPlayback();

    uint8_t frame_index;
    uint8_t frame_tick;
    bool facing_left;
};
