#include "rtos_tasks.h"

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "render.h"
#include "player.h"
#include "level.h"
#include "debug.h"
#include "serial_input.h"
#include "input.h"
#include "camera.h"
#include "config.h"
#include "game_state.h"
#include "spawn.h"

#include <cstdint>
#include <freertos/task.h>

// The TFT instance lives in the main sketch (Flash_transparent_PNG.ino).
extern TFT_eSPI tft;

// ---------------------------------------------------------------------------
// Input task: runs on core 0, polls serial input and pushes the latest
// InputState to the queue every ~20 ms.
// ---------------------------------------------------------------------------
void inputTask(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    SerialInput input;

    for (;;) {
        InputState state = input.poll();
        xQueueOverwrite(queue, &state);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ---------------------------------------------------------------------------
// Sim + render task: runs on core 1 at a fixed 60 Hz cadence.
// Full visible-viewport redraw each frame — no player-clear pass needed.
// ---------------------------------------------------------------------------
void simRenderTask(void *pvParameters) {
    SimParams &p = *(SimParams *)pvParameters;
    TickType_t lastWake = xTaskGetTickCount();
    InputState input{};
    const SpawnPoint initial_spawn{32.0f, 0.0f};
    static constexpr uint32_t BLINK_INTERVAL_MS = 100;
    SpawnManager spawn_manager(initial_spawn);
    Player player(tft, initial_spawn.x, initial_spawn.y);
    Renderer renderer(tft);
    Camera camera((float)(LEVEL_COLS * TILE_SIZE));
    GameState game_state;

    Serial.printf("[GAME] Started with %u lives.\n",
                  (unsigned)game_state.lives());

    for (;;) {
        // Drain the queue — keep only the most recent InputState.
        InputState tmp;
        while (xQueueReceive(p.inputQueue, &tmp, 0) == pdTRUE) {
            input = tmp;
        }

        if (game_state.isGameOver()) {
            if (input.jump) {
                game_state.reset();
                camera.reset();
                player.respawn(spawn_manager.initial());
                Serial.printf("[GAME] Restarted with %u lives.\n",
                              (unsigned)game_state.lives());
            } else {
                vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
                continue;
            }
        }

        const uint32_t now_ms = (uint32_t)millis();
        bool invulnerable = game_state.isInvulnerable(now_ms);

#if DEBUG_MODE
        unsigned long t0 = micros();
        float pvx = player.getVX(), pvy = player.getVY();
        float px  = player.getX(),  py  = player.getY();
        bool  pg  = player.isGrounded();
#endif

        int cam_x = camera.offset();

        player.update(level_map, input, cam_x, invulnerable);
        const bool pit_contact =
            player.getY() + Player::H >= (float)SCREEN_H;
        // Future enemy hits should join this same death transition.
        const bool death_detected = pit_contact;

        if (death_detected) {
            const float death_x = player.getX();
            if (game_state.loseLife(now_ms)) {
                Serial.printf("[GAME] DEATH: life lost, %u lives remaining.\n",
                              (unsigned)game_state.lives());

                if (game_state.isGameOver()) {
                    Serial.println("[GAME] GAME OVER. Press 'w' to restart.");
                } else {
                    SpawnPoint spawn = spawn_manager.respawnBehind(
                        level_map, death_x, initial_spawn.y, Player::W);
                    player.respawn(spawn);
                    camera.focusOn(spawn.x);
                    invulnerable = game_state.isInvulnerable(now_ms);
                }
            }
        }

        if (!game_state.isGameOver()) {
            camera.update(player.getX());
            cam_x = camera.offset();
            player.clampLeft((float)cam_x);

            xSemaphoreTake(p.tftMutex, portMAX_DELAY);
            {
              int spr_x = (int)(player.getX() - cam_x) +
                          (Player::W - Player::SPRITE_W) / 2;
              int spr_y = (int)player.getY() + Player::H - Player::SPRITE_H;
              const bool show_player =
                  !invulnerable ||
                  ((now_ms / BLINK_INTERVAL_MS) % 2U == 0U);
              const uint16_t *sprite = show_player ? player_sprite : nullptr;
              renderer.render_frame(level_map, cam_x, sprite,
                                    spr_x, spr_y,
                                    Player::SPRITE_W, Player::SPRITE_H);
            }
            xSemaphoreGive(p.tftMutex);
        }

#if DEBUG_MODE
        if (!game_state.isGameOver() && !pit_contact) {
            debug_frame(player, level_map, cam_x, micros() - t0,
                        pvx, pvy, px, py, pg);
        }
#endif

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
    }
}
