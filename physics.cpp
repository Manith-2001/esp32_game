#include "physics.h"
#include "Arduino.h"

bool Physics::is_solid(const uint8_t level_map[][COLS], int px, int py) {
    if (px < 0 || px >= COLS * TILE_S || py < 0)
        return false;
    if (py >= ROWS * TILE_S)
        return true;
    uint8_t col = px / TILE_S;
    uint8_t row = py / TILE_S;
    return pgm_read_byte(&level_map[row][col]) != 0;
}

bool Physics::check_ground(const uint8_t level_map[][COLS], float x, float y, int w, int h) {
    int left = (int)x;
    int right = (int)(x + w - 1);
    int bottom = (int)(y + h);
    if (bottom >= ROWS * TILE_S) return true;
    for (int px = left; px <= right; px++) {
        if (is_solid(level_map, px, bottom)) return true;
    }
    return false;
}

void Physics::resolve_x(const uint8_t level_map[][COLS], float &x, float y, int w, int h, float &vx) {
    float new_x = x + vx;
    int top = (int)y;
    int bot = (int)(y + h - 1);

    if (vx > 0) {
        int right = (int)(new_x + w - 1);
        for (int py = top; py <= bot; py++) {
            if (is_solid(level_map, right, py)) {
                new_x = (right / TILE_S) * TILE_S - w;
                vx = 0;
                break;
            }
        }
    } else if (vx < 0) {
        int left = (int)new_x;
        for (int py = top; py <= bot; py++) {
            if (is_solid(level_map, left, py)) {
                new_x = ((left / TILE_S) + 1) * TILE_S;
                vx = 0;
                break;
            }
        }
    }

    x = new_x;
}

void Physics::resolve_y(const uint8_t level_map[][COLS], float x, float &y, int w, int h, float &vy, bool &grounded) {
    float new_y = y + vy;
    int left = (int)x;
    int right = (int)(x + w - 1);

    if (vy > 0) {
        int bot = (int)(new_y + h - 1);
        for (int px = left; px <= right; px++) {
            if (is_solid(level_map, px, bot)) {
                new_y = (bot / TILE_S) * TILE_S - h;
                vy = 0;
                grounded = true;
                break;
            }
        }
    } else if (vy < 0) {
        int top = (int)new_y;
        for (int px = left; px <= right; px++) {
            if (is_solid(level_map, px, top)) {
                new_y = ((top / TILE_S) + 1) * TILE_S;
                vy = 0;
                break;
            }
        }
    }

    y = new_y;
}
