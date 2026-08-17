// This example renders a png file that is stored in a FLASH array
// using the PNGdec library (available via library manager).

// Note: The PNGDEC required lots of RAM to work (~40kbytes) so
// this sketch is will not run on smaller memory processors (e.g.
// ESP8266, STM32F103 etc.)

// The example png is encoded as ARGB 8 bits per pixel with indexed colour
// It was created using GIMP and has a transparent background area.

// Image files can be converted to arrays using the tool here:
// https://notisrac.github.io/FileToCArray/
// To use this tool:
//   1. Drag and drop PNG image file on "Browse..." button
//   2. Tick box "Treat as binary"
//   3. Click "Convert"
//   4. Click "Save as file" and move the header file to sketch folder
//      (alternatively use the "Copy to clipboard" and paste into a new tab)
//   5. Open the sketch in IDE
//   6. Include the header file containing the array (SpongeBob.h in this example)

// Include the PNG decoder library, available via the IDE library manager
#include <PNGdec.h>

// Include image array
#include "config.h"
#include "SpongeBob.h"
#include "assets/piskel.h"
#include "otp_screen.h"
#include "player.h"
#include "level.h"
#include "tiles.h"
#include "debug.h"
#include "serial_input.h"
#include "rtos_tasks.h"
#include "gpio_test_task.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Include the TFT library - see https://github.com/Bodmer/TFT_eSPI for library information
#include "SPI.h"
#include <TFT_eSPI.h>       // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

// Shared parameters for the sim+render task; lifetime spans the whole sketch.
static SimParams g_simParams;

//====================================================================================
//                                    Setup
//====================================================================================
void setup() {
  Serial.begin(115200);

  // Create IPC primitives before any task is running.
  g_simParams.tftMutex = xSemaphoreCreateMutex();
  g_simParams.inputQueue = xQueueCreate(1, sizeof(InputState));

  // Initialise the TFT and draw the OTP gate once.
  // All TFT access is guarded by the mutex, including setup-time rendering.
  xSemaphoreTake(g_simParams.tftMutex, portMAX_DELAY);
  tft.begin();
  tft.setRotation(3);
  if (!tft.initDMA()) {
    Serial.println("TFT DMA init failed");
  }
  OtpScreen otp_screen(tft);
  otp_screen.render();
  xSemaphoreGive(g_simParams.tftMutex);

  debug_init();

  // Start the real-time tasks.
  // Sim+render on core 1 (APP_CPU), higher priority; input on core 0 (PRO_CPU).
  xTaskCreatePinnedToCore(simRenderTask, "simRender", 4096, &g_simParams, 2, NULL, 1);
  xTaskCreatePinnedToCore(inputTask,     "input",     1024, g_simParams.inputQueue, 1, NULL, 0);
  xTaskCreatePinnedToCore(gpioTestTask,  "gpioTest",  4096, NULL, 1, NULL, 0);

  Serial.println("\r\nInitialisation done.");
}

//====================================================================================
//                                    Loop
//====================================================================================
void loop() {
  // All work is done by FreeRTOS tasks; the Arduino loopTask sleeps forever.
  for (;;) {
    vTaskDelay(portMAX_DELAY);
  }
}
