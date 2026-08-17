#pragma once

#include "TFT_eSPI.h"
#include "input.h"

#include <cstdint>

class OtpScreen {
public:
    explicit OtpScreen(TFT_eSPI &tft);

    bool update(const InputState &input);
    void render();
    bool isUnlocked() const;

private:
    static constexpr uint8_t OTP_LENGTH = 4;
    static constexpr uint8_t DIGIT_COUNT = 10;

    bool matchesExpectedOtp() const;
    void resetEntry();

    TFT_eSPI &tft;
    uint8_t selected_digit;
    uint8_t entered_digits[OTP_LENGTH];
    uint8_t entered_count;
    bool unlocked;
    bool show_error;
    bool left_was_pressed;
    bool right_was_pressed;
    bool jump_was_pressed;
};
