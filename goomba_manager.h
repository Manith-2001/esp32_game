#pragma once

#include <cstdint>

#include "config.h"
#include "goomba.h"

class GoombaManager {
public:
    static constexpr uint8_t MIN_GOOMBAS = 4;
    static constexpr uint8_t MAX_GOOMBAS = 8;

    GoombaManager();

    void randomize(const uint8_t level_map[][LEVEL_COLS], float avoid_x);

    uint8_t count() const;
    Goomba &at(uint8_t index);
    const Goomba &at(uint8_t index) const;

private:
    static constexpr float MIN_HORIZONTAL_SPACING = 28.0f;
    static constexpr float PLAYER_START_CLEARANCE = 4.0f * TILE_SIZE;

    bool chooseSpawn(const uint8_t level_map[][LEVEL_COLS],
                     float avoid_x, float &spawn_x, float &spawn_y) const;
    bool isValidCandidate(const uint8_t level_map[][LEVEL_COLS],
                          int row, int col, int x_offset,
                          float avoid_x) const;
    bool isTooClose(float candidate_x, float candidate_y) const;

    Goomba goombas[MAX_GOOMBAS];
    uint8_t active_count;
};
