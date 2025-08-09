#pragma once
#include "esphome/components/display/display.h"
#include "esphome/components/image/image.h"
#include "ui_colors.h"

namespace ui {

inline void draw_monthday(esphome::display::Display &it,
                          esphome::font::Font *font_month,
                          esphome::font::Font *font_day,
                          int x, int y,
                          auto now) {
    char month_text[4];
    now.strftime(month_text, sizeof(month_text), "%b");  // e.g., "Jun"
    std::string month_string(month_text);
    std::transform(month_string.begin(), month_string.end(), month_string.begin(), ::toupper); // JUN
    it.printf(  x + 10 + (5 * 0), y, font_month, PINK, "%c", month_string[0]); // J
    it.printf(  x + 10 + (5 * 1), y, font_month, PINK, "%c", month_string[1]); // U
    it.printf(  x + 10 + (5 * 2), y, font_month, PINK, "%c", month_string[2]); // N
    it.strftime(x               , y, font_day, PURPLE, "%d", now); // 08 <<- day
};

inline void draw_hms(esphome::display::Display &it,
                     esphome::font::Font *font_hourminute,
                     esphome::font::Font *font_second,
                     int x, int y,
                     auto now) {
    it.strftime(x     , y    , font_hourminute, ORANGE, "%H:%M", now);
    it.strftime(x + 87, y + 5, font_second,     ORANGE, "%S",    now);
};

} // end namespace ui
