#include "game_state.h"

GameState::GameState()
    : remaining_lives(STARTING_LIVES), phase(Phase::PLAYING),
      invulnerable_until_ms(0), respawn_protected(false) {}

bool GameState::loseLife(uint32_t now_ms) {
    if (phase == Phase::GAME_OVER || remaining_lives == 0 ||
        isInvulnerable(now_ms))
        return false;

    --remaining_lives;
    if (remaining_lives == 0) {
        phase = Phase::GAME_OVER;
        invulnerable_until_ms = 0;
        respawn_protected = false;
    } else {
        invulnerable_until_ms = now_ms + RESPAWN_INVULNERABILITY_MS;
        respawn_protected = true;
    }

    return true;
}

void GameState::reset() {
    remaining_lives = STARTING_LIVES;
    phase = Phase::PLAYING;
    invulnerable_until_ms = 0;
    respawn_protected = false;
}

uint8_t GameState::lives() const {
    return remaining_lives;
}

bool GameState::isGameOver() const {
    return phase == Phase::GAME_OVER;
}

bool GameState::isInvulnerable(uint32_t now_ms) const {
    return respawn_protected &&
           static_cast<int32_t>(invulnerable_until_ms - now_ms) > 0;
}
