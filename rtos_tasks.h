#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Parameters shared with the sim+render task.  Lifetime matches the sketch
// (global in the .ino), so the task can read it safely after setup() returns.
struct SimParams {
    QueueHandle_t    inputQueue;
    SemaphoreHandle_t tftMutex;
};

// FreeRTOS task entry points.
// pvParameters for inputTask is the QueueHandle_t it writes to.
// pvParameters for simRenderTask is a SimParams*.
void inputTask(void *pvParameters);
void simRenderTask(void *pvParameters);
