#pragma once
#include "esphome/components/display/display.h"
#include "esphome/components/image/image.h"
#include "esphome/core/time.h"
#include "ui_colors.h"
#include "ui_shared.h"

namespace ui {

    inline void draw_month(esphome::display::Display &it,
                           esphome::font::Font *font,
                           const Coord anchor,
                           esphome::ESPTime now) {
        static int last_mon = -1;
        static Box prev_box;
        static const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        if (last_mon == now.month) return;
        char buf[4];
        snprintf(buf, sizeof(buf), "%s", months[now.month-1]);
        last_mon = now.month;
        wipe_and_print(&it,
                       font,
                       anchor.x,
                       anchor.y,
                       buf,
                       esphome::display::TextAlign::LEFT,
                       BLACK,
                       PINK,
                       prev_box
                       );
    };

    inline void draw_day(esphome::display::Display &it,
                         esphome::font::Font *font,
                         const Coord anchor,
                         esphome::ESPTime now) {
        static int last_d = -1;
        static Box prev_box;
        if (now.day_of_month == last_d) return;
        char buf[3];
        snprintf(buf, sizeof(buf), "%02d", now.day_of_month);

        last_d = now.day_of_month;
        wipe_and_print(&it,
                       font,
                       anchor.x,
                       anchor.y,
                       buf,
                       esphome::display::TextAlign::LEFT,
                       BLACK,
                       PURPLE,
                       prev_box
                       );
    };

    inline void draw_time_hm(esphome::display::Display &it,
                             esphome::font::Font *font,
                             const Coord anchor,
                             const esphome::ESPTime now) {
        static int last_h = -1, last_m = -1;
        static Box prev_box;


        if (now.hour == last_h && now.minute == last_m) return;
        char buf[6];                       // "HH:MM" + NUL
        snprintf(buf, sizeof(buf), "%02d:%02d", now.hour, now.minute);
        last_h = now.hour; last_m = now.minute;

        wipe_and_print(&it,
                       font,
                       anchor.x,
                       anchor.y,
                       buf,
                       esphome::display::TextAlign::LEFT,
                       BLACK,
                       ORANGE,
                       prev_box
                       );
    }

    inline void draw_time_s(esphome::display::Display &it,
                            esphome::font::Font *font,
                            const Coord anchor,
                            const esphome::ESPTime now) {
        static int last_s = -1;
        static Box prev_box;


        if (now.second == last_s) return;
        char buf[3];                       // "HH:MM" + NUL
        snprintf(buf, sizeof(buf), "%02d", now.second);
        last_s = now.second;

        wipe_and_print(&it,
                       font,
                       anchor.x,
                       anchor.y,
                       buf,
                       esphome::display::TextAlign::LEFT,
                       BLACK,
                       ORANGE,
                       prev_box
                       );
    }

} // end namespace ui
