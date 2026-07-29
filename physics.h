#pragma once
#include <cstdint>

namespace Physics {
static constexpr uint8_t COLS = 20;
static constexpr uint8_t ROWS = 15;
static constexpr uint8_t TILE_S = 16;

bool is_solid(const uint8_t level_map[][COLS], int px, int py);
void resolve_x(const uint8_t level_map[][COLS], float &x, float y, int w, int h, float &vx);
void resolve_y(const uint8_t level_map[][COLS], float x, float &y, int w, int h, float &vy, bool &grounded);
bool check_ground(const uint8_t level_map[][COLS], float x, float y, int w, int h);
}
