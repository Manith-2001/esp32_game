#include "otp_screen.h"

#include "config.h"

namespace {
constexpr char EXPECTED_OTP[] = "1008";

constexpr int TITLE_Y = 42;
constexpr int ENTRY_Y = 82;
constexpr int ENTRY_WIDTH = 36;
constexpr int ENTRY_HEIGHT = 38;
constexpr int ENTRY_GAP = 12;
constexpr int ENTRY_RADIUS = 6;

constexpr int DIGIT_Y = 145;
constexpr int DIGIT_FIRST_CENTER_X = 30;
constexpr int DIGIT_STEP = 29;
constexpr int DIGIT_BOX_WIDTH = 24;
constexpr int DIGIT_BOX_HEIGHT = 34;
constexpr int DIGIT_BOX_RADIUS = 4;

constexpr int MOVE_HINT_Y = 190;
constexpr int SELECT_HINT_Y = 212;
}  // namespace

OtpScreen::OtpScreen(TFT_eSPI &tft)
    : tft(tft),
      selected_digit(0),
      entered_count(0),
      unlocked(false),
      show_error(false),
      left_was_pressed(false),
      right_was_pressed(false),
      jump_was_pressed(false) {
    resetEntry();
}

bool OtpScreen::update(const InputState &input) {
    const bool left_pressed = input.left && !left_was_pressed;
    const bool right_pressed = input.right && !right_was_pressed;
    const bool jump_pressed = input.jump && !jump_was_pressed;
    bool changed = false;

    if (!unlocked) {
        if (left_pressed || right_pressed || jump_pressed) {
            changed = show_error;
            show_error = false;
        }

        if (left_pressed != right_pressed) {
            if (left_pressed) {
                selected_digit = selected_digit == 0
                                     ? DIGIT_COUNT - 1
                                     : selected_digit - 1;
            } else {
                selected_digit = selected_digit == DIGIT_COUNT - 1
                                     ? 0
                                     : selected_digit + 1;
            }
            changed = true;
        }

        if (jump_pressed && entered_count < OTP_LENGTH) {
            entered_digits[entered_count] = selected_digit;
            entered_count++;
            changed = true;

            if (entered_count == OTP_LENGTH) {
                if (matchesExpectedOtp()) {
                    unlocked = true;
                } else {
                    resetEntry();
                    show_error = true;
                }
            }
        }
    }

    left_was_pressed = input.left;
    right_was_pressed = input.right;
    jump_was_pressed = input.jump;

    return changed;
}

void OtpScreen::render() {
    const int entry_total_width =
        OTP_LENGTH * ENTRY_WIDTH + (OTP_LENGTH - 1) * ENTRY_GAP;
    const int entry_start_x =
        (static_cast<int>(SCREEN_W) - entry_total_width) / 2;

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("ENTER OTP", SCREEN_W / 2, TITLE_Y, 4);

    for (uint8_t slot = 0; slot < OTP_LENGTH; slot++) {
        const int x = entry_start_x +
                      slot * (ENTRY_WIDTH + ENTRY_GAP);
        const bool active = slot == entered_count;
        const uint16_t border_color = active ? TFT_YELLOW : TFT_DARKGREY;

        tft.fillRoundRect(x, ENTRY_Y, ENTRY_WIDTH, ENTRY_HEIGHT,
                          ENTRY_RADIUS, TFT_NAVY);
        tft.drawRoundRect(x, ENTRY_Y, ENTRY_WIDTH, ENTRY_HEIGHT,
                          ENTRY_RADIUS, border_color);

        if (slot < entered_count) {
            tft.setTextColor(TFT_WHITE, TFT_NAVY);
            tft.drawString("*", x + ENTRY_WIDTH / 2,
                           ENTRY_Y + ENTRY_HEIGHT / 2, 2);
        }
    }

    for (uint8_t digit = 0; digit < DIGIT_COUNT; digit++) {
        const int center_x = DIGIT_FIRST_CENTER_X + digit * DIGIT_STEP;
        const bool selected = digit == selected_digit;
        const uint16_t background = selected ? TFT_YELLOW : TFT_BLACK;
        const uint16_t text_color = selected ? TFT_BLACK : TFT_WHITE;

        if (selected) {
            tft.fillRoundRect(
                center_x - DIGIT_BOX_WIDTH / 2,
                DIGIT_Y - DIGIT_BOX_HEIGHT / 2,
                DIGIT_BOX_WIDTH,
                DIGIT_BOX_HEIGHT,
                DIGIT_BOX_RADIUS,
                background);
        }

        tft.setTextColor(text_color, background);
        tft.drawNumber(digit, center_x, DIGIT_Y, 2);
    }

    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("LEFT / RIGHT TO MOVE", SCREEN_W / 2,
                   MOVE_HINT_Y, 1);

    tft.setTextColor(show_error ? TFT_RED : TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(show_error ? "INVALID OTP" : "JUMP TO SELECT",
                   SCREEN_W / 2, SELECT_HINT_Y, 1);
}

bool OtpScreen::isUnlocked() const {
    return unlocked;
}

bool OtpScreen::matchesExpectedOtp() const {
    for (uint8_t i = 0; i < OTP_LENGTH; i++) {
        const uint8_t expected_digit =
            static_cast<uint8_t>(EXPECTED_OTP[i] - '0');
        if (entered_digits[i] != expected_digit)
            return false;
    }

    return true;
}

void OtpScreen::resetEntry() {
    selected_digit = 0;
    entered_count = 0;
    for (uint8_t i = 0; i < OTP_LENGTH; i++)
        entered_digits[i] = 0;
}
