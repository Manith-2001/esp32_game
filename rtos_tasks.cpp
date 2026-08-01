#include "rtos_tasks.h"

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "render.h"
#include "player.h"
#include "level.h"
#include "tiles.h"
#include "debug.h"
#include "serial_input.h"
#include "input.h"
#include "camera.h"
#include "config.h"

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
    Player player(tft, 32, 0);
    Renderer renderer(tft);
    Camera camera((float)(LEVEL_COLS * TILE_SIZE));

    for (;;) {
        // Drain the queue — keep only the most recent InputState.
        InputState tmp;
        while (xQueueReceive(p.inputQueue, &tmp, 0) == pdTRUE) {
            input = tmp;
        }

#if DEBUG_MODE
        unsigned long t0 = micros();
        float pvx = player.getVX(), pvy = player.getVY();
        float px  = player.getX(),  py  = player.getY();
        bool  pg  = player.isGrounded();
#endif

        int cam_x = camera.offset();

        player.update(level_map, input, cam_x);
        camera.update(player.getX());
        cam_x = camera.offset();
        player.clampLeft((float)cam_x);

        xSemaphoreTake(p.tftMutex, portMAX_DELAY);
        renderer.render_visible(level_map, cam_x);
        player.draw(cam_x);
        xSemaphoreGive(p.tftMutex);

#if DEBUG_MODE
        debug_frame(player, level_map, cam_x, micros() - t0,
                    pvx, pvy, px, py, pg);
#endif

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
    }
}
