#pragma once
#include <cstdint>

namespace Physics {
static constexpr uint8_t COLS = 20;
static constexpr uint8_t ROWS = 15;
static constexpr uint8_t TILE_S = 16;

// tile index -> solid?  0=empty 1=block 2=grass (solid) 3=cloud (decorative,
// passable)
static constexpr bool SOLID_TILE[] = {false, true, false, false};

bool tile_is_solid(uint8_t tile_index);
bool is_solid(const uint8_t level_map[][COLS], int px, int py);
void resolve_x(const uint8_t level_map[][COLS], float &x, float y, int w, int h,
               float &vx);
void resolve_y(const uint8_t level_map[][COLS], float x, float &y, int w, int h,
               float &vy);
bool check_ground(const uint8_t level_map[][COLS], float x, float y, int w,
                  int h);
} // namespace Physics
