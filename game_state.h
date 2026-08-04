#pragma once

#include <cstdint>

class GameState {
public:
    static constexpr uint8_t STARTING_LIVES = 3;

    enum class Phase : uint8_t {
        PLAYING,
        GAME_OVER,
    };

    GameState();

    void loseLife();
    void reset();

    uint8_t lives() const;
    bool isGameOver() const;

private:
    uint8_t remaining_lives;
    Phase phase;
};
