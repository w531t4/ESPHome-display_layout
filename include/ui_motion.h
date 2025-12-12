#pragma once

#include "esphome/components/display/display.h"
#include "ui_colors.h"

namespace ui {

inline void draw_y_crawler(esphome::display::Display &it, const int x) {
    static uint16_t xpos = 16;
    static uint16_t lastpos = xpos;
    static bool mvright = true;
    const uint16_t min = 0;
    const uint16_t max = 31;
    if (mvright && xpos < max) {
        xpos++;
    } else if (mvright && xpos == max) {
        xpos--;
        mvright = false;
    } else if (xpos > min) {
        xpos--;
    } else if (xpos == min) {
        xpos++;
        mvright = true;
    }
    if (lastpos != xpos) {
        it.draw_pixel_at(x, lastpos, BLACK);
    }
    it.draw_pixel_at(x, xpos, GREEN);
    lastpos = xpos;
}
} // namespace ui