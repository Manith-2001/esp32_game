#include "render.h"
#include "Arduino.h"
#include "TFT_eSPI.h"
#include "tiles.h"
#include <cstdint>
#include <cstring>

Renderer::Renderer(TFT_eSPI &tft) : tft(tft) {}

const uint16_t *Renderer::tile_for(uint8_t index) {
  switch (index) {
  case 1:
    return block;
  case 2:
    return grass;
  case 3:
    return cloud;
  default:
    return nullptr;
  }
}

void Renderer::render_level(const uint8_t level_map[][20]) {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      uint8_t tile = pgm_read_byte(&level_map[row][col]);
      if (tile == 0) {
        continue;
      }
      const uint16_t *data = tile_for(tile);
      uint16_t data_mut[TILE_S * TILE_S];
      memcpy(data_mut, data, TILE_S * TILE_S * sizeof(uint16_t));
      for (int i = 0; i < TILE_S * TILE_S; i++) {
        if (data_mut[i] == 0x0000) {
          data_mut[i] = 0x7D86;
        }
      }
      tft.pushImage(col * TILE_S, row * TILE_S, TILE_S, TILE_S, data_mut);
    }
  }
}
