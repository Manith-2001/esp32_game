#include "menu.h"

#include "config.h"

namespace {
constexpr int MENU_PANEL_X = 16;
constexpr int MENU_PANEL_Y = 16;
constexpr int MENU_PANEL_W = static_cast<int>(SCREEN_W) - 32;
constexpr int MENU_PANEL_H = static_cast<int>(SCREEN_H) - 32;
constexpr int MENU_PANEL_RADIUS = 14;

constexpr int MENU_TITLE_Y = 42;
constexpr int MENU_TITLE_GAP = 6;
constexpr uint8_t MENU_TITLE_FONT = 4;
constexpr uint8_t MENU_HEART_SCALE = 3;

constexpr int MENU_START_BOX_X = 56;
constexpr int MENU_START_BOX_Y = 122;
constexpr int MENU_START_BOX_W = 208;
constexpr int MENU_START_BOX_H = 40;
constexpr int MENU_START_BOX_RADIUS = 8;
constexpr int MENU_START_Y = 134;
constexpr int MENU_HINT_Y = 179;

constexpr uint8_t HEART_WIDTH = 5;
constexpr uint8_t HEART_HEIGHT = 5;
constexpr uint8_t HEART_PATTERN[HEART_HEIGHT] = {
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
};

void draw_pixel_heart(TFT_eSPI &tft, int x, int y, uint8_t scale,
                      uint16_t color) {
  for (uint8_t row = 0; row < HEART_HEIGHT; row++) {
    for (uint8_t col = 0; col < HEART_WIDTH; col++) {
      const uint8_t bit = 1U << (HEART_WIDTH - 1 - col);
      if ((HEART_PATTERN[row] & bit) != 0) {
        tft.fillRect(x + col * scale, y + row * scale, scale, scale, color);
      }
    }
  }
}

void draw_title(TFT_eSPI &tft) {
  const char *left = "mubu";
  const char *right = "manith";
  const int heart_width = HEART_WIDTH * MENU_HEART_SCALE;
  const int left_width = tft.textWidth(left, MENU_TITLE_FONT);
  const int right_width = tft.textWidth(right, MENU_TITLE_FONT);
  const int title_width = left_width + MENU_TITLE_GAP + heart_width +
                          MENU_TITLE_GAP + right_width;
  const int title_x = (static_cast<int>(SCREEN_W) - title_width) / 2;
  const int heart_x = title_x + left_width + MENU_TITLE_GAP;
  const int right_x = heart_x + heart_width + MENU_TITLE_GAP;
  const int heart_y = MENU_TITLE_Y + 5;

  tft.setTextDatum(TL_DATUM);

  tft.setTextColor(TFT_MAROON, TFT_PURPLE);
  tft.drawString(left, title_x + 2, MENU_TITLE_Y + 2, MENU_TITLE_FONT);
  tft.drawString(right, right_x + 2, MENU_TITLE_Y + 2, MENU_TITLE_FONT);
  draw_pixel_heart(tft, heart_x + 2, heart_y + 2, MENU_HEART_SCALE,
                   TFT_MAROON);

  tft.setTextColor(TFT_WHITE, TFT_PURPLE);
  tft.drawString(left, title_x, MENU_TITLE_Y, MENU_TITLE_FONT);
  tft.drawString(right, right_x, MENU_TITLE_Y, MENU_TITLE_FONT);
  draw_pixel_heart(tft, heart_x, heart_y, MENU_HEART_SCALE, TFT_RED);
}
}

MainMenu::MainMenu(TFT_eSPI &tft) : tft(tft) {}

void MainMenu::render(uint16_t background_color) {
  tft.fillScreen(background_color);
  tft.fillRoundRect(MENU_PANEL_X, MENU_PANEL_Y, MENU_PANEL_W, MENU_PANEL_H,
                    MENU_PANEL_RADIUS, TFT_PURPLE);
  tft.drawRoundRect(MENU_PANEL_X, MENU_PANEL_Y, MENU_PANEL_W, MENU_PANEL_H,
                    MENU_PANEL_RADIUS, TFT_PINK);
  tft.drawRoundRect(MENU_PANEL_X + 4, MENU_PANEL_Y + 4,
                    MENU_PANEL_W - 8, MENU_PANEL_H - 8,
                    MENU_PANEL_RADIUS - 4, TFT_WHITE);

  draw_pixel_heart(tft, MENU_PANEL_X + 18, MENU_PANEL_Y + 16, 2, TFT_PINK);
  draw_pixel_heart(tft, MENU_PANEL_X + MENU_PANEL_W - 28,
                   MENU_PANEL_Y + 16, 2, TFT_PINK);
  draw_pixel_heart(tft, MENU_PANEL_X + 18,
                   MENU_PANEL_Y + MENU_PANEL_H - 28, 2, TFT_RED);
  draw_pixel_heart(tft, MENU_PANEL_X + MENU_PANEL_W - 28,
                   MENU_PANEL_Y + MENU_PANEL_H - 28, 2, TFT_RED);

  draw_title(tft);

  tft.fillRoundRect(MENU_START_BOX_X, MENU_START_BOX_Y, MENU_START_BOX_W,
                    MENU_START_BOX_H, MENU_START_BOX_RADIUS, TFT_MAROON);
  tft.drawRoundRect(MENU_START_BOX_X, MENU_START_BOX_Y, MENU_START_BOX_W,
                    MENU_START_BOX_H, MENU_START_BOX_RADIUS, TFT_PINK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_MAROON);
  tft.drawString("> START GAME", SCREEN_W / 2, MENU_START_Y, 2);
  tft.setTextColor(TFT_WHITE, TFT_PURPLE);
  tft.drawString("PRESS JUMP TO START", SCREEN_W / 2, MENU_HINT_Y, 1);
}
