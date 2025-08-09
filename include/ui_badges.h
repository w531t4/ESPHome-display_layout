#pragma once
// #warning "USING ui_badges.h FROM <your path here>"
#include "esphome.h"
#include "esphome/components/font/font.h"
#include "esphome/components/display/display.h"
#include "ui_colors.h"

namespace ui {

inline void draw_hi_current_low_temp(esphome::display::Display &it,
                                     esphome::font::Font *font,
                                     int x,
                                     int y,
                                     float high,
                                     float current,
                                     float low
                                     ) {
     it.printf(x, y + (11*0), font, RED,  "%.0f", high);
     it.printf(x, y + (11*1), font, TEAL, "%.0f", current);
     it.printf(x, y + (11*2), font, BLUE, "%.0f", low);
};

inline void draw_badge(esphome::display::Display &it,
                       esphome::font::Font *font,
                       int x, int y,           // top-left corner
                       const char *text,       // e.g. "12"
                       int pad_x = 6, int pad_y = 2, int radius = 6) {
    int x1, y1, tw, th;
    it.get_text_bounds(0, 0, text, font,
                       esphome::display::TextAlign::TOP_LEFT,
                       &x1, &y1, &tw, &th);

    const int w = tw + pad_x * 2;
    const int h = th + pad_y * 2;

    // Clamp radius to avoid geometry issues
    if (radius * 2 > h) {
        radius = h / 2;
    }

    // Draw center bar (full height)
    it.filled_rectangle(x + radius, y, w - 2 * radius, h+1, ORANGE);

    // Draw left and right end-caps as full circles (overlaps rectangle, no seams)
    it.filled_circle(x + radius, y + h / 2, radius, ORANGE);
    it.filled_circle(x + w - radius - 1, y + h / 2, radius, ORANGE);

    // Text
    it.print(x + w / 2, y + h / 2, font, BLACK,
             esphome::display::TextAlign::CENTER, text);
}

// Convenience: draw numeric badge in top-right if count > 0
inline void draw_update_badge_top_right(esphome::display::Display &it,
                                        esphome::font::Font *font,
                                        int count, int margin = 4) {
    if (count <= 0) return;
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", count);
    int x1, y1, tw, th;
    it.get_text_bounds(0, 0, buf, font, esphome::display::TextAlign::TOP_LEFT, &x1, &y1, &tw, &th);
    const int pad_x = 6, pad_y = 2, radius = 6;
    const int w = tw + pad_x * 2;
    const int x = it.get_width() - w - margin;
    const int y = margin;
    draw_badge(it, font, x, y, buf, pad_x, pad_y, radius);
    }

} // namespace ui
