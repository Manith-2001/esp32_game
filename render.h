#pragma once
#include "Arduino.h"
#include "TFT_eSPI.h"
#include <cstdint>
#include "config.h"

struct RenderSprite {
  const uint16_t *pixels;
  int x;
  int y;
  int width;
  int height;
  bool flip_x;
};

class Renderer {
public:
  Renderer(TFT_eSPI &tft);
  void render_frame(const uint8_t level_map[][LEVEL_COLS], int cam_x,
                    const RenderSprite *sprites, uint8_t sprite_count);

private:
  static constexpr uint8_t COLS = LEVEL_COLS;
  static constexpr uint8_t ROWS = LEVEL_ROWS;
  static constexpr uint8_t TILE_S = TILE_SIZE;
  const uint16_t *tile_for(uint8_t index);
  TFT_eSPI &tft;
};
