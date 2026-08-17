#include "victory_screen.h"

#include "config.h"

namespace {
constexpr int PANEL_X = 16;
constexpr int PANEL_Y = 16;
constexpr int PANEL_W = static_cast<int>(SCREEN_W) - 32;
constexpr int PANEL_H = static_cast<int>(SCREEN_H) - 32;
constexpr int PANEL_RADIUS = 14;

constexpr int TITLE_Y = 54;
constexpr int MESSAGE_LINE_1_Y = 105;
constexpr int MESSAGE_LINE_2_Y = 130;
constexpr int MESSAGE_LINE_3_Y = 155;
constexpr int HINT_Y = 190;
}  // namespace

VictoryScreen::VictoryScreen(TFT_eSPI &tft) : tft(tft) {}

void VictoryScreen::render() {
    tft.fillScreen(TFT_NAVY);
    tft.fillRoundRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, PANEL_RADIUS,
                      TFT_MAROON);
    tft.drawRoundRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, PANEL_RADIUS,
                      TFT_PINK);
    tft.drawRoundRect(PANEL_X + 4, PANEL_Y + 4, PANEL_W - 8, PANEL_H - 8,
                      PANEL_RADIUS - 4, TFT_WHITE);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_MAROON);
    tft.drawString("GAME OVER", SCREEN_W / 2, TITLE_Y, 4);

    tft.setTextColor(TFT_WHITE, TFT_MAROON);
    tft.drawString("After that, Manith and", SCREEN_W / 2,
                   MESSAGE_LINE_1_Y, 2);
    tft.drawString("Mubu lived happily", SCREEN_W / 2,
                   MESSAGE_LINE_2_Y, 2);
    tft.drawString("ever after.", SCREEN_W / 2,
                   MESSAGE_LINE_3_Y, 2);

    tft.setTextColor(TFT_YELLOW, TFT_MAROON);
    tft.drawString("PRESS W TO RESTART", SCREEN_W / 2, HINT_Y, 1);
}
