#include "player.h"
#include "physics.h"
#include "tiles.h"

Player::Player(TFT_eSPI &tft, float x, float y)
    : tft(tft), x(x), y(y), prev_x(x), prev_y(y), vx(0), vy(0),
      grounded(false) {}

void Player::update(const uint8_t level_map[][20], const InputState &input) {
  float multiplier = grounded ? 1.0f : AIR_CONTROL;
  vx = (input.right - input.left) * SPEED * multiplier;

  if (input.jump && grounded)
    vy = -JUMP_VELOCITY;

  vy += GRAVITY;
  if (vy > MAX_VY)
    vy = MAX_VY;

  Physics::resolve_x(level_map, x, y, W, H, vx);
  Physics::resolve_y(level_map, x, y, W, H, vy);

  if (x < 0) {
    x = 0;
    vx = 0;
  }
  if (x + W > (float)tft.width()) {
    x = tft.width() - W;
    vx = 0;
  }
  if (y < 0) {
    y = 0;
    vy = 0;
  }

  grounded = Physics::check_ground(level_map, x, y, W, H);

  prev_x = x;
  prev_y = y;
}

void Player::draw() { tft.fillRect((int)x, (int)y, W, H, TFT_RED); }

void Player::clear(Renderer &renderer, const uint8_t level_map[][20]) {
  renderer.render_region(level_map, (int)prev_x, (int)prev_y, W, H);
}
