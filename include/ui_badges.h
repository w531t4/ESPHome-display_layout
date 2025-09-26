#pragma once
// #warning "USING ui_badges.h FROM <your path here>"
#include <algorithm>
#include <initializer_list>
#include <string>
#include "esphome.h"
#include "esphome/components/font/font.h"
#include "esphome/components/display/display.h"
#include "ui_colors.h"
#include "ui_shared.h"

namespace ui {
    inline void draw_badge(esphome::display::Display &it,
                           esphome::font::Font *font,
                           const Coord anchor,           // top-left corner
                           const char *text,       // e.g. "12"
                           Box &prev_box,
                           int pad_x = 6, int pad_y = 2, int radius = 6
                           ) {
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
        Box filled_rect = {anchor.x + radius, anchor.y, w - 2 * radius, h+1};
        // Draw center bar (full height)
        it.filled_rectangle(filled_rect.x1, filled_rect.y1, filled_rect.w, filled_rect.h, ORANGE);

        // Draw left and right end-caps as full circles (overlaps rectangle, no seams)
        Box filled_circle1 = {anchor.x, (anchor.y + h / 2) - radius, radius*2, radius*2};
        it.filled_circle(anchor.x + radius, anchor.y + h / 2, radius, ORANGE);
        Box filled_circle2 = {(anchor.x + w - radius - 1) - radius, (anchor.y + h / 2) - radius, radius*2, radius*2};
        it.filled_circle(anchor.x + w - radius - 1, anchor.y + h / 2, radius, ORANGE);

        // Text
        Box textbox = {anchor.x + w / 2, anchor.y + h / 2, w, h};
        it.print(anchor.x + w / 2, anchor.y + h / 2, font, BLACK,
                 esphome::display::TextAlign::CENTER, text);
        prev_box = enclosing_box({textbox, filled_rect, filled_circle1, filled_circle2}).value();
    }

    // Convenience: draw numeric badge in top-right if count > 0
    inline void draw_update_badge_bottom_right(esphome::display::Display &it,
                                               esphome::font::Font *font,
                                               int count, int margin = 4) {
        static int last_count = -1;
        static Box prev_box;

        if (last_count == count) return;
        if (count <= 0 && prev_box.w > 0) {
            it.filled_rectangle(prev_box.x1, prev_box.y1, prev_box.w, prev_box.h, BLACK);
            if (count == 0) return;
        }
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", count);
        int x1, y1, tw, th;
        it.get_text_bounds(0, 0, buf, font, esphome::display::TextAlign::TOP_LEFT, &x1, &y1, &tw, &th);
        const int pad_x = 6, pad_y = 2, radius = 6;
        const int w = tw + pad_x * 2;
        const int x = it.get_width() - w - margin;
        // const int y = margin; // <-- use this for top
        const int y = it.get_height() - margin - th;
        // draw_badge(it, font, x, y, buf, pad_x, pad_y, radius);
        draw_badge(it, font, ui::Coord(x, y), buf, prev_box, pad_x, pad_y, radius);
    }

    inline void draw_printer_status(esphome::display::Display &it,
                                    esphome::font::Font *font,
                                    const Coord anchor,
                                    auto state) {
        it.printf(anchor.x, anchor.y, font, PINK, "%.0f%s", state, "%");
    };

    // Network TX/RX
    struct NetRXTag {};
    struct NetTXTag {};
    inline void draw_network_throughput(esphome::display::Display &it,
                                        esphome::font::Font *font,
                                        const Coord anchor,
                                        const float rx,
                                        const float tx) {
        clean_draw_float<NetTXTag>(it, font, anchor.x, anchor.y,      tx, RED,  BLACK, esphome::display::TextAlign::RIGHT, "%.0f TX", 8);
        clean_draw_float<NetRXTag>(it, font, anchor.x, anchor.y + 11, rx, TEAL, BLACK, esphome::display::TextAlign::RIGHT, "%.0f RX", 8);
    };

    // PSN
    struct PhilTag {};
    struct NickTag {};
    template <typename Tag>
    inline void draw_psn_status(esphome::display::Display &it,
                                esphome::font::Font *font,
                                const Coord anchor, const std::string t, const std::string fmt) {
        clean_draw_string<Tag>(it, font, anchor.x, anchor.y, t, GREEN, BLACK, esphome::display::TextAlign::RIGHT, fmt, 25);
    }

    // TWITCH CHAT
    struct TCSlotOneTag {};
    struct TCSlotTwoTag {};
    struct TCSlotThreeTag {};

    template <typename Tag>
    inline void draw_twitchchat_slot(esphome::display::Display &it,
                                     esphome::font::Font *font,
                                     const Coord anchor, const std::string t, const int max_length) {
        clean_draw_string<Tag>(it, font, anchor.x, anchor.y, t, YELLOW, BLACK, esphome::display::TextAlign::LEFT, "%s", max_length);
    }
} // namespace ui
