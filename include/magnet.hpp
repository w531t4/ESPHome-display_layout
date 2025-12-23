// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>

enum class Magnet : uint8_t {
    LEFT,
    RIGHT,
    AUTO,
    COUNT // keep last to know how many
};