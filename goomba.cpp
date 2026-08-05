#include "goomba.h"

#include "physics.h"

Goomba::Goomba(float spawn_x, float spawn_y)
    : spawn_x(spawn_x),
      spawn_y(spawn_y),
      x(spawn_x),
      y(spawn_y),
      previous_y(spawn_y),
      vx(SPEED),
      vy(0.0f),
      alive(true) {}

void Goomba::reset() {
    x = spawn_x;
    y = spawn_y;
    previous_y = spawn_y;
    vx = SPEED;
    vy = 0.0f;
    alive = true;
}

void Goomba::stomp() {
    alive = false;
    vx = 0.0f;
    vy = 0.0f;
}

bool Goomba::isAlive() const {
    return alive;
}

bool Goomba::hasGroundAt(const uint8_t level_map[][LEVEL_COLS],
                         float test_x) const {
    const int foot_y = static_cast<int>(y + H);
    const int leading_x = vx > 0.0f
                              ? static_cast<int>(test_x + W - 1)
                              : static_cast<int>(test_x);

    return Physics::is_solid(level_map, leading_x, foot_y);
}

float Goomba::getX() const {
    return x;
}

float Goomba::getY() const {
    return y;
}

void Goomba::update(const uint8_t level_map[][LEVEL_COLS]) {
    if (!alive)
        return;

    previous_y = y;

    vy += GRAVITY;
    if (vy > MAX_VY)
        vy = MAX_VY;

    const float attempted_vx = vx;
    const bool grounded = Physics::is_solid(
        level_map, static_cast<int>(x + W / 2), static_cast<int>(y + H));

    // Turn around before walking off a platform edge. This keeps the enemy
    // patrolling instead of disappearing into the first gap it reaches.
    if (grounded && !hasGroundAt(level_map, x + vx))
        vx = -vx;

    Physics::resolve_x(level_map, x, y, W, H, vx);

    // Physics sets vx to zero when a solid wall is hit. Reverse direction
    // instead of stopping permanently.
    if (vx == 0.0f && attempted_vx != 0.0f)
        vx = -attempted_vx;

    if (x < 0.0f) {
        x = 0.0f;
        vx = SPEED;
    }

    const float level_width = static_cast<float>(LEVEL_COLS * TILE_SIZE);
    if (x + W > level_width) {
        x = level_width - W;
        vx = -SPEED;
    }

    Physics::resolve_y(level_map, x, y, W, H, vy);

    // Below-level space is intentionally non-solid so the Goomba can fall
    // into pits without damaging the player.
    if (y + H >= static_cast<float>(SCREEN_H))
        alive = false;
}

bool Goomba::collidesWith(float player_x, float player_y,
                          int player_w, int player_h) const {
    if (!alive)
        return false;

    return player_x < x + W &&
           player_x + player_w > x &&
           player_y < y + H &&
           player_y + player_h > y;
}

bool Goomba::isStompedBy(float player_x, float player_y,
                         float player_previous_y, float player_vy,
                         int player_w, int player_h) const {
    if (!alive || player_vy <= 0.0f)
        return false;

    const bool horizontal_overlap =
        player_x < x + W &&
        player_x + player_w > x;

    if (!horizontal_overlap)
        return false;

    const float previous_player_bottom = player_previous_y + player_h;
    const float current_player_bottom = player_y + player_h;

    // The player's feet crossed the Goomba's top while moving downward.
    return previous_player_bottom <= previous_y &&
           current_player_bottom >= y;
}
