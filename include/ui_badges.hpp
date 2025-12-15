#pragma once
#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
#include "ui_colors.hpp"
#include "ui_shared.hpp"

namespace ui {
inline void draw_printer_status(esphome::display::Display &it,
                                esphome::font::Font *font, const Coord anchor,
                                auto state) {
    it.printf(anchor.x, anchor.y, font, PINK, "%.0f%s", state, "%");
};

} // namespace ui
