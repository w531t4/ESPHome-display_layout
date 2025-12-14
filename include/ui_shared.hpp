#pragma once
#include "esphome/components/display/display.h"
#include "ui_colors.hpp"
#include <algorithm>
#include <limits>
#include <optional>
#include <vector>

namespace ui {

struct Box {
    int x1 = -1, y1 = -1, w = 0, h = 0;
};
struct Coord {
    int x;
    int y;
    constexpr Coord(int x_, int y_) : x{x_}, y{y_} {}
};
inline bool box_valid(const Box &b) { return b.w > 0 && b.h > 0; }
inline int box_x2(const Box &b) { return b.x1 + b.w; } // exclusive
inline int box_y2(const Box &b) { return b.y1 + b.h; } // exclusive

inline std::optional<Box> enclosing_box(const std::vector<Box> &boxes) {
    int min_x = std::numeric_limits<int>::max();
    int min_y = std::numeric_limits<int>::max();
    int max_x = std::numeric_limits<int>::min();
    int max_y = std::numeric_limits<int>::min();
    bool any = false;

    for (const auto &b : boxes) {
        if (!box_valid(b))
            continue;
        any = true;
        if (b.x1 < min_x)
            min_x = b.x1;
        if (b.y1 < min_y)
            min_y = b.y1;
        if (box_x2(b) > max_x)
            max_x = box_x2(b);
        if (box_y2(b) > max_y)
            max_y = box_y2(b);
    }

    if (!any)
        return std::nullopt;

    Box out;
    out.x1 = min_x;
    out.y1 = min_y;
    out.w = max_x - min_x; // exclusive -> width
    out.h = max_y - min_y; // exclusive -> height
    return out;
}
inline std::optional<Box> enclosing_box(std::initializer_list<Box> list) {
    return enclosing_box(std::vector<Box>(list));
}

inline std::optional<Box> enclosing_box(const Box &a, const Box &b) {
    return enclosing_box({a, b});
}

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
inline void wipe_and_print(esphome::display::Display *it,
                           esphome::font::Font *font, int x, int y, char *buf,
                           esphome::display::TextAlign align,
                           esphome::Color blank_color,
                           esphome::Color font_color, Box &prev_box) {
    mywipe(it, prev_box, blank_color);
    myprint(it, font, x, y, buf, align, font_color, prev_box);
}

// must create unique (see below) when used
// struct LowTempTag {};
template <typename Tag>
inline void
clean_draw_float(esphome::display::Display &it, esphome::font::Font *font,
                 int x, int y, float value, esphome::Color font_color,
                 esphome::Color blank_color, esphome::display::TextAlign align,
                 const std::string fmtstring = "%0.f", const int bufsize = 4) {
    static float last = -400.0;
    static Box prev_box;

    if (value == last)
        return;

    char buf[bufsize];
    snprintf(buf, sizeof(buf), fmtstring.c_str(), value);
    last = value;

    wipe_and_print(&it, font, x, y, buf, align, blank_color, font_color,
                   prev_box);
}

template <typename Tag>
inline void
clean_draw_string(esphome::display::Display &it, esphome::font::Font *font,
                  int x, int y, const std::string value,
                  esphome::Color font_color, esphome::Color blank_color,
                  esphome::display::TextAlign align,
                  const std::string fmtstring = "%s", const int bufsize = 4) {
    static std::string last = "preset value";
    static Box prev_box;

    if (value == last)
        return;

    char buf[bufsize];
    snprintf(buf, sizeof(buf), fmtstring.c_str(), value.c_str());
    last = value;

    wipe_and_print(&it, font, x, y, buf, align, blank_color, font_color,
                   prev_box);
}

template <typename Tag>
inline void clean_draw_int(
    esphome::display::Display &it, esphome::font::Font *font, int x, int y,
    const int value, esphome::Color font_color,
    esphome::display::TextAlign align = esphome::display::TextAlign::LEFT,
    esphome::Color blank_color = BLACK, const std::string fmtstring = "%0.f",
    const int bufsize = 4, const int init_last_value = -1) {
    static int last = init_last_value;
    static Box prev_box;

    if (value == last)
        return;

    char buf[bufsize];
    snprintf(buf, sizeof(buf), fmtstring.c_str(), value);
    last = value;

    wipe_and_print(&it, font, x, y, buf, align, blank_color, font_color,
                   prev_box);
}

inline bool txt_sensor_has_healthy_state(
    esphome::homeassistant::HomeassistantTextSensor *ts) {
    return (ts->has_state() && ts->state != "unknown" &&
            ts->state != "unavailable");
}
} // namespace ui