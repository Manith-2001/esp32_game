#pragma once

// === DEBUG SYSTEM ==========================================================
//
// Set DEBUG_MODE to 1 to enable real-time telemetry over Serial (115200 baud).
// When disabled (0), all debug code compiles away — zero runtime cost.
//
// Sub-options (only meaningful when DEBUG_MODE=1):
//   DEBUG_COMPACT    — 1-line per-frame summary (position, velocity, corners)
//   DEBUG_SURROUND   — 3x3 tile grid around player on state changes
//   DEBUG_COLLISION  — detailed report when velocity changes (what tile hit)
//
// Output format legend:
//   .=empty  B=block  G=grass  C=cloud  #=OOB  ==screen bottom ground
// ==========================================================================

#define DEBUG_MODE 1

#if DEBUG_MODE
  #define DEBUG_COMPACT   0
  #define DEBUG_SURROUND  1
  #define DEBUG_COLLISION 1
#endif

#include <cstdint>

class Player;

#if DEBUG_MODE
void debug_init();
void debug_frame(const Player &p, const uint8_t level_map[][20],
                 unsigned long dt_us, float prev_vx, float prev_vy,
                 float prev_x, float prev_y, bool prev_grounded);
#else
#define debug_init() ((void)0)
#define debug_frame(p, lm, dt, pvx, pvy, px, py, pg) ((void)0)
#endif
