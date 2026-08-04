#include "game_state.h"

GameState::GameState()
    : remaining_lives(STARTING_LIVES), phase(Phase::PLAYING) {}

void GameState::loseLife() {
    if (phase == Phase::GAME_OVER || remaining_lives == 0)
        return;

    --remaining_lives;
    if (remaining_lives == 0)
        phase = Phase::GAME_OVER;
}

void GameState::reset() {
    remaining_lives = STARTING_LIVES;
    phase = Phase::PLAYING;
}

uint8_t GameState::lives() const {
    return remaining_lives;
}

bool GameState::isGameOver() const {
    return phase == Phase::GAME_OVER;
}
