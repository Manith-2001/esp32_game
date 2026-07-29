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

void Renderer::draw_tile(uint8_t tile, int col, int row) {
  int x = col * TILE_S;
  int y = row * TILE_S;
  if (tile == 0) {
    tft.fillRect(x, y, TILE_S, TILE_S, BG_COLOR);
    return;
  }
  const uint16_t *data = tile_for(tile);
  uint16_t data_mut[TILE_S * TILE_S];
  memcpy(data_mut, data, TILE_S * TILE_S * sizeof(uint16_t));
  for (int i = 0; i < TILE_S * TILE_S; i++) {
    if (data_mut[i] == 0x0000) {
      data_mut[i] = BG_SWAP(BG_COLOR);
    }
  }
  tft.pushImage(x, y, TILE_S, TILE_S, data_mut);
}

void Renderer::render_level(const uint8_t level_map[][20]) {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      uint8_t tile = pgm_read_byte(&level_map[row][col]);
      draw_tile(tile, col, row);
    }
  }
}

void Renderer::render_region(const uint8_t level_map[][20], int x, int y, int w, int h) {
  if (w <= 0 || h <= 0) return;
  int start_col = x / TILE_S;
  int end_col = (x + w - 1) / TILE_S;
  int start_row = y / TILE_S;
  int end_row = (y + h - 1) / TILE_S;

  if (start_col < 0) start_col = 0;
  if (start_row < 0) start_row = 0;
  if (end_col >= COLS) end_col = COLS - 1;
  if (end_row >= ROWS) end_row = ROWS - 1;

  for (int row = start_row; row <= end_row; row++) {
    for (int col = start_col; col <= end_col; col++) {
      uint8_t tile = pgm_read_byte(&level_map[row][col]);
      draw_tile(tile, col, row);
    }
  }
}
