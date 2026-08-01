#pragma once
#include "TFT_eSPI.h"
#include "render.h"
#include "input.h"

class Player {
public:
    Player(TFT_eSPI &tft, float x, float y);

    void update(const uint8_t level_map[][20], const InputState &input);
    void draw();
    void clear(Renderer &renderer, const uint8_t level_map[][20]);

    float getX() const     { return x; }
    float getY() const     { return y; }
    float getVX() const    { return vx; }
    float getVY() const    { return vy; }
    bool  isGrounded() const { return grounded; }

private:
    static constexpr int W = 10;
    static constexpr int H = 10;
    static constexpr float GRAVITY = 0.4f;
    static constexpr float MAX_VY = 10.0f;
    static constexpr float SPEED = 1.0f;
    static constexpr float JUMP_VELOCITY = 7.0f;
    static constexpr float AIR_CONTROL = 0.7f;

    TFT_eSPI &tft;
    float x, y;
    float prev_x, prev_y;
    float vx, vy;
    bool grounded;
};
