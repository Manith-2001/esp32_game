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
// Drains the latest input, clears the old player rectangle, updates physics,
// draws the new rectangle, and prints debug telemetry.
// ---------------------------------------------------------------------------
void simRenderTask(void *pvParameters) {
    SimParams &p = *(SimParams *)pvParameters;
    TickType_t lastWake = xTaskGetTickCount();
    InputState input{};
    Player player(tft, 10, 0);
    Renderer renderer(tft);

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

        xSemaphoreTake(p.tftMutex, portMAX_DELAY);
        player.clear(renderer, level_map);
        player.update(level_map, input);
        player.draw();
        xSemaphoreGive(p.tftMutex);

#if DEBUG_MODE
        debug_frame(player, level_map, micros() - t0, pvx, pvy, px, py, pg);
#endif

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(16));
    }
}
