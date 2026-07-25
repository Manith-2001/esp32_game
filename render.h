#include "Arduino.h"
#include "TFT_eSPI.h"
#include <cstdint>

class Renderer {
public:
  Renderer(TFT_eSPI &tft);
  void render_level(const uint8_t level_map[][20]);

private:
  static constexpr uint8_t COLS = 20;
  static constexpr uint8_t ROWS = 15;
  static constexpr uint8_t TILE_S = 16;
  TFT_eSPI &tft;
};
