#pragma once
#include "TFT_eSPI.h"
#include "config.h"
#include "input.h"
#include "player_sprite.h"
#include "spawn.h"

class Player {
public:
    static constexpr int W = 10;
    static constexpr int H = 10;
    static constexpr int SPRITE_W = 24;
    static constexpr int SPRITE_H = 24;

    Player(TFT_eSPI &tft, float x, float y);

    void update(const uint8_t level_map[][LEVEL_COLS], const InputState &input,
                int cam_left, bool allow_pit_escape);
    void respawn(const SpawnPoint &spawn);

    float getX() const     { return x; }
    float getY() const     { return y; }
    float getVX() const    { return vx; }
    float getVY() const    { return vy; }
    bool  isGrounded() const { return grounded; }
    void  clampLeft(float left_bound);

private:
    static constexpr float GRAVITY = 0.65f;
    static constexpr float MAX_VY = 14.0f;
    static constexpr float SPEED = 2.0f;
    static constexpr float JUMP_VELOCITY = 9.0f;
    static constexpr float GROUND_ACCELERATION = 0.4f;
    static constexpr float GROUND_FRICTION = 0.4f;
    static constexpr float AIR_CONTROL = 0.7f;

    TFT_eSPI &tft;
    float x, y;
    float prev_x, prev_y;
    float vx, vy;
    bool grounded;
};
