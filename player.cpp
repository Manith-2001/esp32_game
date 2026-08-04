#include "player.h"
#include "physics.h"

Player::Player(TFT_eSPI &tft, float x, float y)
    : tft(tft), x(x), y(y), prev_x(x), prev_y(y), vx(0), vy(0),
      grounded(false) {}

void Player::respawn(float spawn_x, float spawn_y) {
  x = spawn_x;
  y = spawn_y;
  prev_x = x;
  prev_y = y;
  vx = 0;
  vy = 0;
  grounded = false;
}

void Player::update(const uint8_t level_map[][LEVEL_COLS],
                    const InputState &input, int cam_left) {
  clampLeft((float)cam_left);

  const int direction = (input.right ? 1 : 0) - (input.left ? 1 : 0);
  const float acceleration = grounded
                                 ? GROUND_ACCELERATION
                                 : GROUND_ACCELERATION * AIR_CONTROL;

  if (direction != 0) {
    const float target_vx = direction * SPEED;

    // Approach the target instead of replacing vx, so jumps inherit momentum.
    if (vx < target_vx) {
      vx += acceleration;
      if (vx > target_vx) vx = target_vx;
    } else if (vx > target_vx) {
      vx -= acceleration;
      if (vx < target_vx) vx = target_vx;
    }
  } else if (grounded) {
    // Airborne movement has no drag: releasing a direction preserves momentum.
    if (vx > 0) {
      vx -= GROUND_FRICTION;
      if (vx < 0) vx = 0;
    } else if (vx < 0) {
      vx += GROUND_FRICTION;
      if (vx > 0) vx = 0;
    }
  }

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
