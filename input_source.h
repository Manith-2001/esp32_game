#pragma once

#include "gpio_input.h"
#include "serial_input.h"

// Change this alias to SerialInput to use UART controls instead.
using ActiveInput = GpioInput;
