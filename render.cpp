#include "render.h"
#include "Arduino.h"
#include "TFT_eSPI.h"
#include "tiles.h"
#include <cstdint>

Renderer::Renderer(TFT_eSPI &tft) : tft(tft) {}

void Renderer::render_level(const uint8_t level_map[][20]) {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      uint8_t tile = pgm_read_byte(&level_map[row][col]);
      if (tile == 0) {
        continue;
      }
      const uint16_t *data = block;
      tft.pushImage(col * TILE_S, row * TILE_S, TILE_S, TILE_S, data);
    }
  }
}
