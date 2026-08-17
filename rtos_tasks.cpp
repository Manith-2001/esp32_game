#include "rtos_tasks.h"

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "render.h"
#include "player.h"
#include "goomba.h"
#include "goomba_manager.h"
#include "goomba_sprite.h"
#include "level.h"
#include "debug.h"
#include "input_source.h"
#include "input.h"
#include "camera.h"
#include "config.h"
#include "game_state.h"
#include "spawn.h"
#include "player_animation.h"
#include "cutscene.h"
#include "menu.h"
#include "otp_screen.h"
#include "victory_screen.h"

#include <cstdint>
#include <freertos/task.h>

// The TFT instance lives in the main sketch (Flash_transparent_PNG.ino).
extern TFT_eSPI tft;

static constexpr uint8_t WORLD_SPRITE_CAPACITY =
    GoombaManager::MAX_GOOMBAS + 1;
static constexpr uint8_t CUTSCENE_SPRITE_CAPACITY =
    WORLD_SPRITE_CAPACITY + CutsceneLimits::MAX_STAGE_ACTORS;

static void log_goomba_spawns(const GoombaManager &goombas) {
    Serial.printf("[ENEMY] Spawned %u Goombas.\n",
                  (unsigned)goombas.count());

    for (uint8_t i = 0; i < goombas.count(); i++) {
        const Goomba &goomba = goombas.at(i);
        Serial.printf("[ENEMY] #%u at (%d,%d).\n",
                      (unsigned)i,
                      (int)goomba.getX(),
                      (int)goomba.getY());
    }
}

static uint8_t buildWorldSprites(
    const GoombaManager &goombas,
    const Player &player,
    const PlayerAnimation &player_animation,
    int cam_x,
    bool show_player,
    RenderSprite *out,
    uint8_t max) {
    if (out == nullptr || max == 0)
        return 0;

    const uint8_t active_count = goombas.count();
    const uint8_t goomba_count =
        active_count < GoombaManager::MAX_GOOMBAS
            ? active_count
            : GoombaManager::MAX_GOOMBAS;
    uint8_t sprite_count = 0;

    for (uint8_t i = 0; i < goomba_count && sprite_count < max; i++) {
        const Goomba &goomba = goombas.at(i);
        if (!goomba.isAlive())
            continue;

        out[sprite_count++] = {
            goomba_sprite,
            (int)(goomba.getX() - cam_x) +
                (Goomba::W - Goomba::SPRITE_W) / 2,
            (int)goomba.getY() +
                Goomba::H - Goomba::SPRITE_H,
            Goomba::SPRITE_W,
            Goomba::SPRITE_H,
            false};
    }

    if (show_player && sprite_count < max) {
        out[sprite_count++] = {
            player_animation.pixels(),
            (int)(player.getX() - cam_x) +
                (Player::W - Player::SPRITE_W) / 2,
            (int)player.getY() +
                Player::H - Player::SPRITE_H,
            Player::SPRITE_W,
            Player::SPRITE_H,
            player_animation.flipX()};
    }

    return sprite_count;
}

static void composeCutsceneFrame(
    Renderer &renderer,
    CutscenePlayer &cutscene,
    const GoombaManager &goombas,
    const Player &player,
    const PlayerAnimation &player_animation,
    int cam_x,
    SemaphoreHandle_t tft_mutex) {
    RenderSprite sprites[CUTSCENE_SPRITE_CAPACITY];
    const CutsceneBackground background = cutscene.background();
    uint8_t sprite_count = 0;

    if (background == CutsceneBackground::REGULAR_LEVEL) {
        sprite_count = buildWorldSprites(
            goombas,
            player,
            player_animation,
            cam_x,
            true,
            sprites,
            CUTSCENE_SPRITE_CAPACITY);
    }

    sprite_count += cutscene.stageSprites(
        sprites + sprite_count,
        CUTSCENE_SPRITE_CAPACITY - sprite_count);

    xSemaphoreTake(tft_mutex, portMAX_DELAY);
    if (background == CutsceneBackground::FLAT_GROUND)
        renderer.render_flat_ground(sprites, sprite_count);
    else
        renderer.render_frame(level_map, cam_x, sprites, sprite_count);
    tft.dmaWait();
    cutscene.renderOverlay();
    xSemaphoreGive(tft_mutex);
}

// ---------------------------------------------------------------------------
// Input task: runs on core 0, polls the selected input source and pushes the
// latest InputState to the queue every ~20 ms.
// ---------------------------------------------------------------------------
void inputTask(void *pvParameters) {
    QueueHandle_t queue = (QueueHandle_t)pvParameters;
    ActiveInput input;

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
    PlayerAnimation player_animation;
    GoombaManager goombas;
    Renderer renderer(tft);
    Camera camera((float)(LEVEL_COLS * TILE_SIZE));
    GameState game_state;
    CutscenePlayer cutscene(tft);
    VictoryScreen victory_screen(tft);
    OtpScreen otp_screen(tft);
    MainMenu main_menu(tft);
    bool authenticated = false;
    bool game_started = false;
    bool start_armed = false;
    bool ending_cutscene = false;
    bool victory_restart_armed = false;

    randomSeed(static_cast<unsigned long>(micros()));

    for (;;) {
        // Drain the queue — keep only the most recent InputState.
        InputState tmp;
        while (xQueueReceive(p.inputQueue, &tmp, 0) == pdTRUE) {
            input = tmp;
        }

        if (!authenticated) {
            const bool screen_changed = otp_screen.update(input);

            if (!otp_screen.isUnlocked()) {
                if (screen_changed) {
                    xSemaphoreTake(p.tftMutex, portMAX_DELAY);
                    otp_screen.render();
                    xSemaphoreGive(p.tftMutex);
                }

                vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
                continue;
            }

            authenticated = true;
            start_armed = false;
            input = InputState{};

            xSemaphoreTake(p.tftMutex, portMAX_DELAY);
            main_menu.render(TFT_MAROON);
            xSemaphoreGive(p.tftMutex);
            continue;
        }

        if (!game_started) {
            if (!input.jump)
                start_armed = true;

            if (!input.jump || !start_armed) {
                vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
                continue;
            }

            game_started = true;
            goombas.randomize(level_map, initial_spawn.x);
            Serial.printf("[GAME] Started with %u lives.\n",
                          (unsigned)game_state.lives());
            log_goomba_spawns(goombas);
            cutscene.start(&CUTSCENE_INTRO);

            // Do not carry the menu input into the first gameplay frame.
            input = InputState{};

            // Let the input task provide a real release snapshot before the
            // cutscene can arm its skip handler.
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
            continue;
        }

        if (game_state.isVictory()) {
            if (!input.jump)
                victory_restart_armed = true;

            if (input.jump && victory_restart_armed) {
                game_state.reset();
                camera.reset();
                player.respawn(spawn_manager.initial());
                player_animation.reset();
                goombas.randomize(level_map, initial_spawn.x);
                cutscene.resetTriggers();
                ending_cutscene = false;
                victory_restart_armed = false;
                Serial.printf("[GAME] Restarted with %u lives.\n",
                              (unsigned)game_state.lives());
                log_goomba_spawns(goombas);
            } else {
                vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
                continue;
            }
        } else if (game_state.isGameOver()) {
            if (input.jump) {
                game_state.reset();
                camera.reset();
                player.respawn(spawn_manager.initial());
                player_animation.reset();
                goombas.randomize(level_map, initial_spawn.x);
                cutscene.resetTriggers();
                Serial.printf("[GAME] Restarted with %u lives.\n",
                              (unsigned)game_state.lives());
                log_goomba_spawns(goombas);
            } else {
                vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
                continue;
            }
        }

        const uint32_t now_ms = (uint32_t)millis();
        bool invulnerable = game_state.isInvulnerable(now_ms);

        if (cutscene.isActive()) {
            if (cutscene.update(input)) {
                composeCutsceneFrame(renderer, cutscene, goombas, player,
                                     player_animation, camera.offset(),
                                     p.tftMutex);
            }

            if (!cutscene.isActive() && ending_cutscene) {
                ending_cutscene = false;
                game_state.complete();
                victory_restart_armed = false;

                xSemaphoreTake(p.tftMutex, portMAX_DELAY);
                tft.dmaWait();
                victory_screen.render();
                xSemaphoreGive(p.tftMutex);
            }

            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
            continue;
        }

#if DEBUG_MODE
        unsigned long t0 = micros();
        float pvx = player.getVX(), pvy = player.getVY();
        float px  = player.getX(),  py  = player.getY();
        bool  pg  = player.isGrounded();
#endif

        int cam_x = camera.offset();

        player.update(level_map, input, cam_x, invulnerable);
        for (uint8_t i = 0; i < goombas.count(); i++)
            goombas.at(i).update(level_map);

        bool goomba_contact = false;
        int stomped_index = -1;

        // Resolve stomps first so a stomp always wins over side contact with
        // another Goomba in the same frame.
        for (uint8_t i = 0; i < goombas.count(); i++) {
            Goomba &goomba = goombas.at(i);
            if (!goomba.isAlive())
                continue;

            if (goomba.isStompedBy(
                player.getX(),
                player.getY(),
                player.getPreviousY(),
                player.getVY(),
                Player::W,
                Player::H)) {
                stomped_index = i;
                break;
            }
        }

        if (stomped_index >= 0) {
            Goomba &goomba = goombas.at(static_cast<uint8_t>(stomped_index));
            const float goomba_top = goomba.getY();

            goomba.stomp();
            player.bounceFromEnemy(goomba_top);

            Serial.println("[ENEMY] GOOMBA STOMPED.");
        } else {
            for (uint8_t i = 0; i < goombas.count(); i++) {
                const Goomba &goomba = goombas.at(i);
                if (!goomba.isAlive())
                    continue;

                if (goomba.collidesWith(
                    player.getX(),
                    player.getY(),
                    Player::W,
                    Player::H)) {
                    goomba_contact = true;
                    break;
                }
            }
        }

        const bool pit_contact =
            player.getY() + Player::H >= (float)SCREEN_H;
        // Invulnerable players may overlap an enemy without losing another
        // life during the protection window.
        const bool death_detected =
            pit_contact || (goomba_contact && !invulnerable);

        if (death_detected) {
            const float death_x = player.getX();
            const char *death_reason =
                pit_contact ? "PIT" : "GOOMBA";

            if (game_state.loseLife(now_ms)) {
                Serial.printf("[GAME] DEATH: %s, life lost, %u lives remaining.\n",
                              death_reason,
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

        if (!death_detected && !game_state.isGameOver()) {
            cutscene.maybeTrigger(player.getX());
            if (cutscene.completesGame())
                ending_cutscene = true;
        }

        const int input_direction =
            (input.right ? 1 : 0) - (input.left ? 1 : 0);
        player_animation.update(
            player.getVX(), player.getVY(), player.isGrounded(),
            input_direction);

        if (!game_state.isGameOver()) {
            camera.update(player.getX());
            cam_x = camera.offset();
            player.clampLeft((float)cam_x);

            xSemaphoreTake(p.tftMutex, portMAX_DELAY);
            {
              RenderSprite sprites[WORLD_SPRITE_CAPACITY];
              const bool show_player =
                  !invulnerable ||
                  ((now_ms / BLINK_INTERVAL_MS) % 2U == 0U);

              const uint8_t sprite_count =
                  buildWorldSprites(
                      goombas,
                      player,
                      player_animation,
                      cam_x,
                      show_player,
                      sprites,
                      WORLD_SPRITE_CAPACITY);

              renderer.render_frame(level_map, cam_x,
                                    sprites, sprite_count);
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
