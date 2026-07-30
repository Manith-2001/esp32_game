
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
#include "SpongeBob.h"
#include "assets/piskel.h"
#include "render.h"
#include "player.h"
#include "level.h"
#include "tiles.h"
#include "debug.h"
#include "serial_input.h"

PNG png;  // PNG decoder instance
// Compilation error: invalid conversion from 'void (*)(PNGDRAW*)' {aka 'void (*)(png_draw_tag*)'} to 'int (*)(PNGDRAW*)' {aka 'int (*)(png_draw_tag*)'} [-fpermissive]
#define MAX_IMAGE_WIDTH 320  // Sets rendering line buffer lengths, adjust for your images

// Include the TFT library - see https://github.com/Bodmer/TFT_eSPI for library information
#include "SPI.h"
#include <TFT_eSPI.h>       // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
Renderer renderer(tft);
Player player(tft, 10, 0);
SerialInput input;

// Position variables must be global (PNGdec does not handle position coordinates)
int16_t xpos = 0;
int16_t ypos = 120;

//====================================================================================
//                                    Setup
//====================================================================================
void setup() {
  Serial.begin(115200);

  // Initialise the TFT
  tft.begin();
  tft.setRotation(1);

  // Draw the static level once; the game loop only updates the moving player.
  tft.fillScreen(BG_COLOR);
  renderer.render_level(level_map);

  debug_init();

  Serial.println("\r\nInitialisation done.");
}

//====================================================================================
//                                    Loop
//====================================================================================
void loop() {
#if DEBUG_MODE
  unsigned long t0 = micros();
  float pvx = player.getVX(), pvy = player.getVY();
  float px  = player.getX(),  py  = player.getY();
  bool  pg  = player.isGrounded();
#endif

  player.clear(renderer, level_map);
  player.update(level_map, input.poll());
  player.draw();

#if DEBUG_MODE
  debug_frame(player, level_map, micros() - t0, pvx, pvy, px, py, pg);
#endif
}
