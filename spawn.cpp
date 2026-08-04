#include "spawn.h"

#include "physics.h"

SpawnManager::SpawnManager(SpawnPoint initial_spawn,
                           uint8_t backtrack_tiles)
    : initial_spawn(initial_spawn), backtrack_tiles(backtrack_tiles) {}

SpawnPoint SpawnManager::initial() const {
    return initial_spawn;
}

SpawnPoint SpawnManager::respawnBehind(
    const uint8_t level_map[][LEVEL_COLS], float death_x, float spawn_y,
    int entity_width) const {
    if (entity_width < 1)
        entity_width = 1;

    int death_col = (int)death_x / TILE_SIZE;
    if (death_col < 0)
        death_col = 0;
    if (death_col >= LEVEL_COLS)
        death_col = LEVEL_COLS - 1;

    int candidate_col = death_col - backtrack_tiles;
    if (candidate_col < 0)
        candidate_col = 0;

    for (int column = candidate_col; column >= 0; column--) {
        SpawnPoint candidate = pointAtColumn(column, spawn_y, entity_width);
        if (hasSupport(level_map, candidate.x, entity_width))
            return candidate;
    }

    return pointAtColumn(0, spawn_y, entity_width);
}

bool SpawnManager::hasSupport(const uint8_t level_map[][LEVEL_COLS],
                              float x, int entity_width) const {
    const int left = (int)x;
    const int right = left + entity_width - 1;

    for (int px = left; px <= right; px++) {
        bool supported = false;
        for (int row = 1; row < LEVEL_ROWS; row++) {
            if (Physics::is_solid(level_map, px, row * TILE_SIZE)) {
                supported = true;
                break;
            }
        }
        if (!supported)
            return false;
    }

    return true;
}

SpawnPoint SpawnManager::pointAtColumn(int column, float y,
                                       int entity_width) const {
    int x_offset = (TILE_SIZE - entity_width) / 2;
    if (x_offset < 0)
        x_offset = 0;

    float x = (float)(column * TILE_SIZE + x_offset);
    const float max_x = (float)(LEVEL_COLS * TILE_SIZE - entity_width);
    if (x > max_x)
        x = max_x;
    if (x < 0)
        x = 0;

    return {x, y};
}
