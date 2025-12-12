#pragma once
#include "esphome/components/display/display.h"
#include "esphome/components/image/image.h"
#include "ui_colors.h"
#include <string>
#include <unordered_map>

namespace ui {

struct IconPair {
    esphome::image::Image *day = nullptr;
    esphome::image::Image *night = nullptr;
};

inline std::unordered_map<std::string, IconPair> &icon_registry() {
    static std::unordered_map<std::string, IconPair> reg;
    return reg;
}

// Call this from YAML (on_setup) to wire IDs into C++
inline void register_icon(const std::string &state, esphome::image::Image *day,
                          esphome::image::Image *night) {
    icon_registry()[state] = IconPair{day, night};
}

inline bool is_night_hour(int hour, int night_start = 21, int night_end = 6) {
    // Night if hour >= night_start OR hour < night_end (wraps midnight)
    return (hour >= night_start) || (hour < night_end);
}

// The following contains a post which outlines the possible weather states
// https://community.home-assistant.io/t/met-no-weather-condition-possible-states/405353/6
inline void draw_weather_image(esphome::display::Display &it,
                               const Coord anchor, const std::string &state,
                               int this_hour, int night_start = 21,
                               int night_end = 6) {
    const int img_width = 32, img_height = 32;
    static std::string last_state = "junk_state";
    if (last_state == state)
        return;

    auto itf = icon_registry().find(state);
    if (itf == icon_registry().end())
        return;

    const bool night = is_night_hour(this_hour, night_start, night_end);
    esphome::image::Image *img = night ? itf->second.night : itf->second.day;
    if (!img)
        return;

    it.image(anchor.x, anchor.y, img, COLOR_ON, COLOR_OFF); // draw
    last_state = state;
}

struct HighTempTag {};
struct CurrentTempTag {};
struct LowTempTag {};
inline void draw_hi_current_low_temp(esphome::display::Display &it,
                                     esphome::font::Font *font,
                                     const Coord anchor, float high,
                                     float current, float low) {
    clean_draw_float<HighTempTag>(it, font, anchor.x, anchor.y + (11 * 0), high,
                                  RED, BLACK,
                                  esphome::display::TextAlign::LEFT);
    clean_draw_float<CurrentTempTag>(it, font, anchor.x, anchor.y + (11 * 1),
                                     current, TEAL, BLACK,
                                     esphome::display::TextAlign::LEFT);
    clean_draw_float<LowTempTag>(it, font, anchor.x, anchor.y + (11 * 2), low,
                                 BLUE, BLACK,
                                 esphome::display::TextAlign::LEFT);
};

} // namespace ui
