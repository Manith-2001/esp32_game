#pragma once

#include <cstdint>

class GameState {
public:
    static constexpr uint8_t STARTING_LIVES = 3;
    static constexpr uint32_t RESPAWN_INVULNERABILITY_MS = 5000;

    enum class Phase : uint8_t {
        PLAYING,
        GAME_OVER,
    };

    GameState();

    bool loseLife(uint32_t now_ms);
    void reset();

    uint8_t lives() const;
    bool isGameOver() const;
    bool isInvulnerable(uint32_t now_ms) const;

private:
    uint8_t remaining_lives;
    Phase phase;
    uint32_t invulnerable_until_ms;
    bool respawn_protected;
};
