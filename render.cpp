#include "render.h"
#include "Arduino.h"
#include "TFT_eSPI.h"
#include "tiles.h"

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

void Renderer::render_frame(const uint8_t level_map[][LEVEL_COLS], int cam_x,
                             const uint16_t *sprite, int spr_x, int spr_y,
                             int spr_w, int spr_h) {
  static uint16_t row_buf[SCREEN_W * TILE_S];

  int start_col = cam_x / TILE_S;
  int end_col = (cam_x + SCREEN_W - 1) / TILE_S;
  if (start_col < 0) start_col = 0;
  if (end_col >= COLS) end_col = COLS - 1;

  const uint16_t bg = BG_SWAP(BG_COLOR);
  const bool has_sprite = (sprite != nullptr && spr_w > 0 && spr_h > 0);

  int spr_r0 = 0, spr_r1 = -1;
  if (has_sprite) {
    spr_r0 = spr_y / TILE_S;
    spr_r1 = (spr_y + spr_h - 1) / TILE_S;
    if (spr_r0 < 0) spr_r0 = 0;
    if (spr_r1 >= ROWS) spr_r1 = ROWS - 1;
  }

  for (int row = 0; row < ROWS; row++) {
    for (int i = 0; i < SCREEN_W * TILE_S; i++)
      row_buf[i] = bg;

    for (int col = start_col; col <= end_col; col++) {
      uint8_t tile = pgm_read_byte(&level_map[row][col]);
      if (tile == 0)
        continue;

      const uint16_t *td = tile_for(tile);
      int tile_sx = col * TILE_S - cam_x;

      for (int ty = 0; ty < TILE_S; ty++) {
        int ry = ty * SCREEN_W;
        for (int tx = 0; tx < TILE_S; tx++) {
          int sx = tile_sx + tx;
          if (sx < 0 || sx >= (int)SCREEN_W)
            continue;
          uint16_t px = td[ty * TILE_S + tx];
          if (px == 0x0000)
            continue;
          row_buf[ry + sx] = px;
        }
      }
    }

    if (has_sprite && row >= spr_r0 && row <= spr_r1) {
      int row_top = row * TILE_S;
      int row_bot = row_top + TILE_S;
      for (int sy = 0; sy < spr_h; sy++) {
        int screen_y = spr_y + sy;
        if (screen_y < row_top) continue;
        if (screen_y >= row_bot) break;
        int buf_row = (screen_y - row_top) * SCREEN_W;
        for (int sx = 0; sx < spr_w; sx++) {
          int buf_x = spr_x + sx;
          if (buf_x < 0 || buf_x >= (int)SCREEN_W)
            continue;
          uint16_t px = sprite[sy * spr_w + sx];
          if (px == 0x0000)
            continue;
          row_buf[buf_row + buf_x] = px;
        }
      }
    }

    tft.pushImage(0, row * TILE_S, SCREEN_W, TILE_S, row_buf);
  }
}
