#pragma once

#include "esphome.h"

enum class Magnet : uint8_t {
    LEFT, RIGHT, AUTO,
    COUNT  // keep last to know how many
};