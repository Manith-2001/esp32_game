#pragma once

#include <cstdint>

#include "config.h"

struct SpawnPoint {
    float x;
    float y;
};

class SpawnManager {
public:
    static constexpr uint8_t DEFAULT_BACKTRACK_TILES = 3;

    SpawnManager(SpawnPoint initial_spawn,
                 uint8_t backtrack_tiles = DEFAULT_BACKTRACK_TILES);

    SpawnPoint initial() const;
    SpawnPoint respawnBehind(const uint8_t level_map[][LEVEL_COLS],
                             float death_x, float spawn_y,
                             int entity_width) const;

private:
    bool hasSupport(const uint8_t level_map[][LEVEL_COLS], float x,
                   int entity_width) const;
    SpawnPoint pointAtColumn(int column, float y, int entity_width) const;

    SpawnPoint initial_spawn;
    uint8_t backtrack_tiles;
};
