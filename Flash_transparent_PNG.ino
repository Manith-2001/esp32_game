
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
#include "level.h"

PNG png;  // PNG decoder instance
// Compilation error: invalid conversion from 'void (*)(PNGDRAW*)' {aka 'void (*)(png_draw_tag*)'} to 'int (*)(PNGDRAW*)' {aka 'int (*)(png_draw_tag*)'} [-fpermissive]
#define MAX_IMAGE_WIDTH 320  // Sets rendering line buffer lengths, adjust for your images

// Include the TFT library - see https://github.com/Bodmer/TFT_eSPI for library information
#include "SPI.h"
#include <TFT_eSPI.h>       // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
Renderer renderer(tft);

// Position variables must be global (PNGdec does not handle position coordinates)
int16_t xpos = 0;
int16_t ypos = 120;

//====================================================================================
//                                    Setup
//====================================================================================
void setup() {
  Serial.begin(115200);
  // Serial.println("\n\n Using the PNGdec library");

  // Initialise the TFT
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_SKYBLUE);
  renderer.render_level(level_map);

  Serial.println("\r\nInitialisation done.");
}

//====================================================================================
//                                    Loop
//====================================================================================
void loop() {
  // uint16_t pngw = 0, pngh = 0;  // To store width and height of image
  //
  // // int16_t rc = png.openFLASH((uint8_t *)bob, sizeof(bob), pngDraw);
  // int16_t background = png.openFLASH((uint8_t *)piskel, sizeof(piskel), pngDraw);
  //
  // if (background == PNG_SUCCESS) {
  //   Serial.println("Successfully opened png file");
  //   pngw = png.getWidth();
  //   pngh = png.getHeight();
  //   Serial.printf("Image metrics: (%d x %d), %d bpp, pixel type: %d\n", pngw, pngh, png.getBpp(), png.getPixelType());
  //
  //   tft.startWrite();
  //   uint32_t dt = millis();
  //   // rc = png.decode(NULL, 0);
  //   Serial.println("Starting write");
  //   background = png.decode(NULL, 0);
  //   Serial.println("Ending write");
  //   tft.endWrite();
  //   Serial.print(millis() - dt);
  //   Serial.println("ms");
  //   tft.endWrite();
  //
  //   // png.close(); // Required for files, not needed for FLASH arrays
  // }
  //
  delay(250);
  //
  // // Randomly change position
  // // xpos = random(tft.width() - pngw);
  // // ypos = random(tft.height() - pngh);
  //
  // // Fill screen with a random colour at random intervals
  // // if (random(100) < 20) tft.fillScreen(random(0x10000));
}
