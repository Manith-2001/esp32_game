#include "gpio_test_task.h"

#include "button_pins.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace {
static constexpr uint32_t POLL_INTERVAL_MS = 20;
}

void gpioTestTask(void *pvParameters) {
    (void)pvParameters;

    // All test pins use external pull-down resistors on the perf board.
    pinMode(ButtonPins::LEFT, INPUT);
    pinMode(ButtonPins::RIGHT, INPUT);
    pinMode(ButtonPins::JUMP, INPUT);

    int last_state_17 = digitalRead(ButtonPins::LEFT);
    int last_state_4 = digitalRead(ButtonPins::RIGHT);
    int last_state_18 = digitalRead(ButtonPins::JUMP);

    for (;;) {
        const int state_17 = digitalRead(ButtonPins::LEFT);
        const int state_4 = digitalRead(ButtonPins::RIGHT);
        const int state_18 = digitalRead(ButtonPins::JUMP);

        if (state_17 != last_state_17) {
            if (state_17 == HIGH) {
                Serial.println("[GPIO TEST] GPIO17 changed to HIGH digitalRead=1");
            } else {
                Serial.println("[GPIO TEST] GPIO17 changed to LOW digitalRead=0");
            }
            last_state_17 = state_17;
        }

        if (state_4 != last_state_4) {
            if (state_4 == HIGH) {
                Serial.println("[GPIO TEST] GPIO4 changed to HIGH digitalRead=1");
            } else {
                Serial.println("[GPIO TEST] GPIO4 changed to LOW digitalRead=0");
            }
            last_state_4 = state_4;
        }

        if (state_18 != last_state_18) {
            if (state_18 == HIGH) {
                Serial.println("[GPIO TEST] GPIO18 changed to HIGH digitalRead=1");
            } else {
                Serial.println("[GPIO TEST] GPIO18 changed to LOW digitalRead=0");
            }
            last_state_18 = state_18;
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }
}
