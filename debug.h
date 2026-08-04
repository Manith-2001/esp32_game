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
//   .=empty  B=block  G=grass  C=cloud  #=OOB  ==below-level death edge
// ==========================================================================

#define DEBUG_MODE 1

#if DEBUG_MODE
  #define DEBUG_COMPACT   1
  #define DEBUG_SURROUND  1
  #define DEBUG_COLLISION 1
#endif

#include <cstdint>
#include "config.h"

class Player;

#if DEBUG_MODE

enum GameEvent : uint8_t {
    EVENT_NONE             = 0,
    EVENT_PLAYER_MOVED     = 1 << 0,
    EVENT_VELOCITY_CHANGE  = 1 << 1,
    EVENT_GROUND_CHANGE    = 1 << 2,
    EVENT_COLLISION        = 1 << 3,
};

void debug_init();
void debug_signal(GameEvent event);
void debug_frame(const Player &p, const uint8_t level_map[][LEVEL_COLS],
                 int cam_x, unsigned long dt_us,
                 float prev_vx, float prev_vy,
                 float prev_x, float prev_y, bool prev_grounded);
#else
#define debug_init() ((void)0)
#define debug_signal(ev) ((void)0)
#define debug_frame(p, lm, cam_x, dt, pvx, pvy, px, py, pg) ((void)0)
#endif
