#include "player.h"
#include "physics.h"
#include "tiles.h"

Player::Player(TFT_eSPI &tft, float x, float y)
    : tft(tft), x(x), y(y), prev_x(x), prev_y(y), vx(SPEED), vy(0), grounded(false) {}

void Player::update(const uint8_t level_map[][20]) {
    vy += GRAVITY;
    if (vy > MAX_VY) vy = MAX_VY;

    Physics::resolve_x(level_map, x, y, W, H, vx);
    Physics::resolve_y(level_map, x, y, W, H, vy);

    if (x < 0)                { x = 0;            vx =  SPEED; }
    if (x + W > tft.width())  { x = tft.width() - W; vx = -SPEED; }
    if (y < 0)                { y = 0;            vy = 0; }

    grounded = Physics::check_ground(level_map, x, y, W, H);

    prev_x = x;
    prev_y = y;
}

void Player::draw() {
    tft.fillRect((int)x, (int)y, W, H, BG_SWAP(TFT_RED));
}

void Player::clear(Renderer &renderer, const uint8_t level_map[][20]) {
    renderer.render_region(level_map, (int)prev_x, (int)prev_y, W, H);
}
