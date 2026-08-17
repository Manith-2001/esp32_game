#pragma once

#include "TFT_eSPI.h"

class VictoryScreen {
public:
    explicit VictoryScreen(TFT_eSPI &tft);

    void render();

private:
    TFT_eSPI &tft;
};
