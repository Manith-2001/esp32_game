#include "gpio_test_task.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace {
static constexpr uint8_t GPIO_TEST_PIN_17 = 17; // Left
static constexpr uint8_t GPIO_TEST_PIN_16 = 16; // Right
static constexpr uint8_t GPIO_TEST_PIN_18 = 18; // Jump
static constexpr uint32_t POLL_INTERVAL_MS = 20;
}

void gpioTestTask(void *pvParameters) {
    (void)pvParameters;

    // All test pins use external pull-down resistors on the perf board.
    pinMode(GPIO_TEST_PIN_17, INPUT);
    pinMode(GPIO_TEST_PIN_16, INPUT);
    pinMode(GPIO_TEST_PIN_18, INPUT);

    int last_state_17 = digitalRead(GPIO_TEST_PIN_17);
    int last_state_16 = digitalRead(GPIO_TEST_PIN_16);
    int last_state_18 = digitalRead(GPIO_TEST_PIN_18);

    for (;;) {
        const int state_17 = digitalRead(GPIO_TEST_PIN_17);
        const int state_16 = digitalRead(GPIO_TEST_PIN_16);
        const int state_18 = digitalRead(GPIO_TEST_PIN_18);

        if (state_17 != last_state_17) {
            if (state_17 == HIGH) {
                Serial.println("[GPIO TEST] GPIO17 changed to HIGH digitalRead=1");
            } else {
                Serial.println("[GPIO TEST] GPIO17 changed to LOW digitalRead=0");
            }
            last_state_17 = state_17;
        }

        if (state_16 != last_state_16) {
            if (state_16 == HIGH) {
                Serial.println("[GPIO TEST] GPIO16 changed to HIGH digitalRead=1");
            } else {
                Serial.println("[GPIO TEST] GPIO16 changed to LOW digitalRead=0");
            }
            last_state_16 = state_16;
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
