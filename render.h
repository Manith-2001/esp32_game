#pragma once
#include "Arduino.h"
#include "TFT_eSPI.h"
#include <cstdint>
#include "config.h"

class Renderer {
public:
  Renderer(TFT_eSPI &tft);
  void render_visible(const uint8_t level_map[][LEVEL_COLS], int cam_x);

private:
  static constexpr uint8_t COLS = LEVEL_COLS;
  static constexpr uint8_t ROWS = LEVEL_ROWS;
  static constexpr uint8_t TILE_S = TILE_SIZE;
  const uint16_t *tile_for(uint8_t index);
  void draw_tile(uint8_t tile, int col, int row, int cam_x);
  TFT_eSPI &tft;
};
