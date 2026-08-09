#include "render.h"
#include "Arduino.h"
#include "TFT_eSPI.h"
#include "tiles.h"

static constexpr uint8_t FLAT_GROUND_FIRST_ROW = 10;
static uint16_t row_buf[2][SCREEN_W * TILE_SIZE];

static void draw_sprites_on_row(uint16_t *pixels, int row,
                                const RenderSprite *sprites,
                                uint8_t sprite_count) {
  if (pixels == nullptr || sprites == nullptr || sprite_count == 0)
    return;

  for (uint8_t sprite_index = 0;
       sprite_index < sprite_count;
       sprite_index++) {
    const RenderSprite &sprite = sprites[sprite_index];

    if (sprite.pixels == nullptr ||
        sprite.width <= 0 ||
        sprite.height <= 0) {
      continue;
    }

    int sprite_row_start = sprite.y / TILE_SIZE;
    int sprite_row_end =
        (sprite.y + sprite.height - 1) / TILE_SIZE;

    if (sprite_row_start < 0)
      sprite_row_start = 0;

    if (sprite_row_end >= SCREEN_ROWS)
      sprite_row_end = SCREEN_ROWS - 1;

    if (row < sprite_row_start || row > sprite_row_end)
      continue;

    const int row_top = row * TILE_SIZE;
    const int row_bottom = row_top + TILE_SIZE;

    for (int sy = 0; sy < sprite.height; sy++) {
      const int screen_y = sprite.y + sy;

      if (screen_y < row_top)
        continue;

      if (screen_y >= row_bottom)
        break;

      const int buffer_row = (screen_y - row_top) * SCREEN_W;

      for (int sx = 0; sx < sprite.width; sx++) {
        const int screen_x = sprite.x + sx;

        if (screen_x < 0 || screen_x >= (int)SCREEN_W)
          continue;

        const int source_x =
            sprite.flip_x ? sprite.width - 1 - sx : sx;
        const uint16_t pixel =
            sprite.pixels[sy * sprite.width + source_x];

        if (pixel == 0x0000)
          continue;

        pixels[buffer_row + screen_x] = pixel;
      }
    }
  }
}

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
                            const RenderSprite *sprites,
                            uint8_t sprite_count) {
  int start_col = cam_x / TILE_S;
  int end_col = (cam_x + SCREEN_W - 1) / TILE_S;
  if (start_col < 0) start_col = 0;
  if (end_col >= COLS) end_col = COLS - 1;

  const uint16_t bg = BG_SWAP(BG_COLOR);
  const bool has_sprites = sprites != nullptr && sprite_count > 0;

  tft.startWrite();

  for (int row = 0; row < ROWS; row++) {
    uint16_t *pixels = row_buf[row & 1];
    for (int i = 0; i < SCREEN_W * TILE_S; i++)
      pixels[i] = bg;

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
          pixels[ry + sx] = px;
        }
      }
    }

    if (has_sprites)
      draw_sprites_on_row(pixels, row, sprites, sprite_count);

    tft.pushImageDMA(0, row * TILE_S, SCREEN_W, TILE_S,
                     static_cast<const uint16_t *>(pixels));
  }

  tft.endWrite();
}

void Renderer::render_flat_ground(const RenderSprite *sprites,
                                  uint8_t sprite_count) {
  const uint16_t bg = BG_SWAP(BG_COLOR);
  const uint16_t *ground_tile = tile_for(1);

  tft.startWrite();

  for (int row = 0; row < ROWS; row++) {
    uint16_t *pixels = row_buf[row & 1];
    for (int i = 0; i < SCREEN_W * TILE_S; i++)
      pixels[i] = bg;

    if (ground_tile != nullptr && row >= FLAT_GROUND_FIRST_ROW) {
      for (int col = 0; col < SCREEN_COLS; col++) {
        for (int ty = 0; ty < TILE_S; ty++) {
          const int buffer_row = ty * SCREEN_W + col * TILE_S;
          const int tile_row = ty * TILE_S;

          for (int tx = 0; tx < TILE_S; tx++) {
            const uint16_t pixel = ground_tile[tile_row + tx];
            if (pixel != 0x0000)
              pixels[buffer_row + tx] = pixel;
          }
        }
      }
    }

    draw_sprites_on_row(pixels, row, sprites, sprite_count);

    tft.pushImageDMA(0, row * TILE_S, SCREEN_W, TILE_S,
                     static_cast<const uint16_t *>(pixels));
  }

  tft.endWrite();
}
