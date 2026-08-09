#pragma once

#include "Arduino.h"
#include "TFT_eSPI.h"
#include <cstdint>

class MainMenu {
public:
  explicit MainMenu(TFT_eSPI &tft);
  void render(uint16_t background_color);

private:
  TFT_eSPI &tft;
};
