#include "menu.h"

#include "config.h"

namespace {
constexpr int MENU_PANEL_X = 24;
constexpr int MENU_PANEL_Y = 24;
constexpr int MENU_PANEL_W = static_cast<int>(SCREEN_W) - 48;
constexpr int MENU_PANEL_H = static_cast<int>(SCREEN_H) - 48;
constexpr int MENU_TITLE_Y = 56;
constexpr int MENU_START_Y = 132;
constexpr int MENU_HINT_Y = 180;
}

MainMenu::MainMenu(TFT_eSPI &tft) : tft(tft) {}

void MainMenu::render(uint16_t background_color) {
  tft.fillScreen(background_color);
  tft.fillRect(MENU_PANEL_X, MENU_PANEL_Y, MENU_PANEL_W, MENU_PANEL_H,
               TFT_BLUE);
  tft.drawRect(MENU_PANEL_X, MENU_PANEL_Y, MENU_PANEL_W, MENU_PANEL_H,
               TFT_YELLOW);
  tft.drawRect(MENU_PANEL_X + 4, MENU_PANEL_Y + 4,
               MENU_PANEL_W - 8, MENU_PANEL_H - 8, TFT_WHITE);

  tft.setTextColor(TFT_RED, TFT_BLUE);
  tft.drawCentreString("GAME PROJ", SCREEN_W / 2 + 2, MENU_TITLE_Y + 2, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.drawCentreString("GAME PROJ", SCREEN_W / 2, MENU_TITLE_Y, 4);

  tft.setTextColor(TFT_YELLOW, TFT_BLUE);
  tft.drawCentreString("> START GAME", SCREEN_W / 2, MENU_START_Y, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.drawCentreString("PRESS JUMP TO START", SCREEN_W / 2, MENU_HINT_Y, 1);
}
