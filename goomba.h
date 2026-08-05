#pragma once

#include <cstdint>

#include "config.h"

class Goomba {
public:
    static constexpr int W = 12;
    static constexpr int H = 12;
    static constexpr int SPRITE_W = 16;
    static constexpr int SPRITE_H = 16;

    explicit Goomba(float spawn_x, float spawn_y);

    void update(const uint8_t level_map[][LEVEL_COLS]);
    void reset();
    void stomp();

    bool isAlive() const;
    bool collidesWith(float player_x, float player_y,
                      int player_w, int player_h) const;
    bool isStompedBy(float player_x, float player_y,
                     float player_previous_y, float player_vy,
                     int player_w, int player_h) const;

    float getX() const;
    float getY() const;

private:
    static constexpr float SPEED = 0.75f;
    static constexpr float GRAVITY = 0.65f;
    static constexpr float MAX_VY = 14.0f;

    bool hasGroundAt(const uint8_t level_map[][LEVEL_COLS],
                     float test_x) const;

    float spawn_x;
    float spawn_y;

    float x;
    float y;
    float previous_y;
    float vx;
    float vy;

    bool alive;
};
