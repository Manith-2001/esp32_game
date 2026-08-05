#include "player.h"
#include "physics.h"

Player::Player(TFT_eSPI &tft, float x, float y)
    : tft(tft), x(x), y(y), prev_x(x), prev_y(y), vx(0), vy(0),
      grounded(false) {}

void Player::respawn(const SpawnPoint &spawn) {
  x = spawn.x;
  y = spawn.y;
  prev_x = x;
  prev_y = y;
  vx = 0;
  vy = 0;
  grounded = false;
}

void Player::bounceFromEnemy(float enemy_top) {
  y = enemy_top - H;
  if (y < 0.0f)
    y = 0.0f;

  vy = -STOMP_BOUNCE_VELOCITY;
  grounded = false;
  prev_y = y;
}

void Player::update(const uint8_t level_map[][LEVEL_COLS],
                    const InputState &input, int cam_left,
                    bool allow_pit_escape) {
  prev_x = x;
  prev_y = y;

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

  const bool at_pit_bottom =
      allow_pit_escape && y + H >= (float)SCREEN_H;
  if (!at_pit_bottom)
    Physics::resolve_x(level_map, x, y, W, H, vx);
  else
    x += vx;
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

  if (allow_pit_escape && y + H >= (float)SCREEN_H) {
    // Keep protected players visible and let them run across the pit floor.
    y = (float)(SCREEN_H - H);
    vy = 0;

    // If they reach the solid bottom row, lift them onto its top surface so
    // the normal collision system can take over on the next frame.
    const int left = (int)x;
    const int right = (int)(x + W - 1);
    for (int px = left; px <= right; px++) {
      if (Physics::is_solid(level_map, px, SCREEN_H - 1)) {
        y = (float)(SCREEN_H - TILE_SIZE - H);
        break;
      }
    }
  }

  grounded = Physics::check_ground(level_map, x, y, W, H);
}

void Player::clampLeft(float left_bound) {
  if (x < left_bound) {
    x = left_bound;
    vx = 0;
  }
}
