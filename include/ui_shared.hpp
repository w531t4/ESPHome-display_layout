// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
#include "esphome/components/homeassistant/text_sensor/homeassistant_text_sensor.h"
#include <algorithm>

namespace ui {

struct Box {
    int x1 = -1, y1 = -1, w = 0, h = 0;
};
struct Coord {
    int x;
    int y;
    constexpr Coord(int x_, int y_) : x{x_}, y{y_} {}
};

inline void mywipe(esphome::display::Display *it, Box &prev_box,
                   esphome::Color blank_color) {
    if (prev_box.w > 0 && prev_box.h > 0) {
        it->filled_rectangle(prev_box.x1, prev_box.y1, prev_box.w + 1,
                             prev_box.h, blank_color);
    }
}

inline void printf_dual(
    esphome::display::Display *it, esphome::font::Font *font, const int x,
    const int y, const char *left_text, esphome::Color left_color,
    const char *right_text, esphome::Color right_color, ui::Box &prev_box,
    const int spacing = 0,
    esphome::display::TextAlign align = esphome::display::TextAlign::TOP_LEFT) {
    /** Print two strings in series like strcat, with different colors
     *
     * @param it Pointer to the display object.
     * @param font Pointer to the font object.
     * @param x The x coordinate of the text alignment anchor point.
     * @param y The y coordinate of the text alignment anchor point.
     * @param left_text Pointer to the start of a c-string representing the
     * first (left-most) text blob.
     * @param left_color Render left_text with this color.
     * @param right_text Pointer to the start of a c-string representing the
     * last (right-most) text blob.
     * @param right_color Render right_text with this color
     * @param prev_box Reference object to place aggregate dimensions into.
     * @param spacing number of pixels to place inbetween left and right
     * objects.
     * @param align Determines how to interpret x, y
     */
    //
    it->printf(x, y, font, left_color, align, "%s",
               left_text); // Draw left part

    int tx1, ty1, tw, th;
    it->get_text_bounds(x, y, left_text, font, align, &tx1, &ty1, &tw,
                        &th); // Compute where to start the right text
    const ui::Box lb{tx1, ty1, tw, th};
    const int left_width = tw;
    const int x2 = x + left_width + spacing;

    it->printf(x2, y, font, right_color, align, "%s",
               right_text); // Draw right part

    it->get_text_bounds(x2, y, right_text, font, align, &tx1, &ty1, &tw, &th);
    const ui::Box rb{tx1, ty1, tw, th};

    const int min_x = std::min(lb.x1, rb.x1);
    const int min_y = std::min(lb.y1, rb.y1);
    const int max_r = std::max(lb.x1 + lb.w, rb.x1 + rb.w);
    const int max_b = std::max(lb.y1 + lb.h, rb.y1 + rb.h);

    prev_box = ui::Box{min_x, min_y, max_r - min_x, max_b - min_y};
}

inline void myprint(esphome::display::Display *it, esphome::font::Font *font,
                    int x, int y, char *buf, esphome::display::TextAlign align,
                    esphome::Color font_color, Box &prev_box) {
    int x1, y1, w, h;
    it->get_text_bounds(x, y, buf, font, align, &x1, &y1, &w, &h);
    it->printf(x, y, font, font_color, align, "%s", buf);
    prev_box = {x1, y1, w, h};
}

inline bool txt_sensor_has_healthy_state(
    esphome::homeassistant::HomeassistantTextSensor *ts) {
    return (ts->has_state() && ts->state != "unknown" &&
            ts->state != "unavailable");
}
} // namespace ui