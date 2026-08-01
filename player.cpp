#include "player.h"
#include "physics.h"
#include "tiles.h"

Player::Player(TFT_eSPI &tft, float x, float y)
    : tft(tft), x(x), y(y), prev_x(x), prev_y(y), vx(0), vy(0),
      grounded(false) {}

void Player::update(const uint8_t level_map[][LEVEL_COLS],
                    const InputState &input, int cam_left) {
  clampLeft((float)cam_left);

  float multiplier = grounded ? 1.0f : AIR_CONTROL;
  vx = (input.right - input.left) * SPEED * multiplier;

  if (input.jump && grounded)
    vy = -JUMP_VELOCITY;

  vy += GRAVITY;
  if (vy > MAX_VY)
    vy = MAX_VY;

  Physics::resolve_x(level_map, x, y, W, H, vx);
  Physics::resolve_y(level_map, x, y, W, H, vy);

  clampLeft((float)cam_left);

  if (x + W > (float)(LEVEL_COLS * TILE_SIZE)) {
    x = (float)(LEVEL_COLS * TILE_SIZE) - W;
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

void Player::clampLeft(float left_bound) {
  if (x < left_bound) {
    x = left_bound;
    vx = 0;
  }
}

void Player::draw(int cam_x) {
  tft.fillRect((int)(x - cam_x), (int)y, W, H, TFT_RED);
}
