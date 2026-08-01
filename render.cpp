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

void Renderer::draw_tile(uint8_t tile, int col, int row, int cam_x) {
  int sx = col * TILE_S - cam_x;
  int sy = row * TILE_S;

  if (sx + TILE_S <= 0 || sx >= (int)SCREEN_W ||
      sy + TILE_S <= 0 || sy >= (int)SCREEN_H)
    return;

  const uint16_t *data = tile_for(tile);
  uint16_t data_mut[TILE_S * TILE_S];
  memcpy(data_mut, data, TILE_S * TILE_S * sizeof(uint16_t));
  for (int i = 0; i < TILE_S * TILE_S; i++) {
    if (data_mut[i] == 0x0000) {
      data_mut[i] = BG_SWAP(BG_COLOR);
    }
  }
  tft.pushImage(sx, sy, TILE_S, TILE_S, data_mut);
}

void Renderer::render_visible(const uint8_t level_map[][LEVEL_COLS], int cam_x) {
  int start_col = cam_x / TILE_S;
  int end_col = (cam_x + SCREEN_W - 1) / TILE_S;

  if (start_col < 0) start_col = 0;
  if (end_col >= COLS) end_col = COLS - 1;

  for (int row = 0; row < ROWS; row++) {
    for (int col = start_col; col <= end_col; col++) {
      uint8_t tile = pgm_read_byte(&level_map[row][col]);
      if (tile == 0) {
        int sx = col * TILE_S - cam_x;
        int sy = row * TILE_S;
        if (sx + TILE_S <= 0 || sx >= (int)SCREEN_W ||
            sy + TILE_S <= 0 || sy >= (int)SCREEN_H)
          continue;
        tft.fillRect(sx, sy, TILE_S, TILE_S, BG_COLOR);
        continue;
      }
      draw_tile(tile, col, row, cam_x);
    }
  }
}
