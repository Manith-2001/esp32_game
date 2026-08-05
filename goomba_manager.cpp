#include "goomba_manager.h"

#include "Arduino.h"
#include "physics.h"

#include <math.h>

GoombaManager::GoombaManager() : active_count(0) {}

void GoombaManager::randomize(const uint8_t level_map[][LEVEL_COLS],
                              float avoid_x) {
    active_count = 0;

    const uint8_t target_count =
        static_cast<uint8_t>(random(MIN_GOOMBAS, MAX_GOOMBAS + 1));

    for (uint8_t i = 0; i < target_count; i++) {
        float spawn_x = 0.0f;
        float spawn_y = 0.0f;

        if (!chooseSpawn(level_map, avoid_x, spawn_x, spawn_y))
            break;

        goombas[active_count].spawnAt(spawn_x, spawn_y);
        active_count++;
    }

    for (uint8_t i = active_count; i < MAX_GOOMBAS; i++)
        goombas[i].stomp();
}

uint8_t GoombaManager::count() const {
    return active_count;
}

Goomba &GoombaManager::at(uint8_t index) {
    return goombas[index];
}

const Goomba &GoombaManager::at(uint8_t index) const {
    return goombas[index];
}

bool GoombaManager::chooseSpawn(
    const uint8_t level_map[][LEVEL_COLS], float avoid_x,
    float &spawn_x, float &spawn_y) const {
    uint16_t valid_count = 0;

    for (int row = 1; row < LEVEL_ROWS; row++) {
        for (int col = 0; col < LEVEL_COLS; col++) {
            for (int x_offset = 0;
                 x_offset <= TILE_SIZE - Goomba::W;
                 x_offset++) {
                if (isValidCandidate(level_map, row, col, x_offset,
                                     avoid_x)) {
                    valid_count++;
                }
            }
        }
    }

    if (valid_count == 0)
        return false;

    const uint16_t selected =
        static_cast<uint16_t>(random(static_cast<long>(valid_count)));
    uint16_t candidate_index = 0;

    for (int row = 1; row < LEVEL_ROWS; row++) {
        for (int col = 0; col < LEVEL_COLS; col++) {
            for (int x_offset = 0;
                 x_offset <= TILE_SIZE - Goomba::W;
                 x_offset++) {
                if (!isValidCandidate(level_map, row, col, x_offset,
                                      avoid_x)) {
                    continue;
                }

                if (candidate_index == selected) {
                    spawn_x = static_cast<float>(col * TILE_SIZE + x_offset);
                    spawn_y = static_cast<float>(row * TILE_SIZE - Goomba::H);
                    return true;
                }

                candidate_index++;
            }
        }
    }

    return false;
}

bool GoombaManager::isValidCandidate(
    const uint8_t level_map[][LEVEL_COLS], int row, int col,
    int x_offset, float avoid_x) const {
    const int support_x = col * TILE_SIZE + x_offset;
    const int support_y = row * TILE_SIZE;
    const float candidate_y = static_cast<float>(support_y - Goomba::H);

    // A Goomba must fit inside one solid tile and stand on its top edge.
    if (!Physics::is_solid(level_map, support_x, support_y))
        return false;
    if (Physics::is_solid(level_map, support_x, support_y - 1))
        return false;

    const float candidate_x = static_cast<float>(support_x);
    if (fabsf(candidate_x - avoid_x) < PLAYER_START_CLEARANCE)
        return false;

    return !isTooClose(candidate_x, candidate_y);
}

bool GoombaManager::isTooClose(float candidate_x, float candidate_y) const {
    for (uint8_t i = 0; i < active_count; i++) {
        const float vertical_distance = candidate_y - goombas[i].getY();
        const float horizontal_distance = candidate_x - goombas[i].getX();

        // Enemies on separate platform heights can share an x coordinate.
        if (fabsf(vertical_distance) < static_cast<float>(Goomba::H) &&
            fabsf(horizontal_distance) < MIN_HORIZONTAL_SPACING) {
            return true;
        }
    }

    return false;
}
