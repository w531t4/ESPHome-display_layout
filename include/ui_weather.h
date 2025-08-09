#pragma once
#include <string>
#include <unordered_map>
#include "esphome/components/display/display.h"
#include "esphome/components/image/image.h"

namespace ui {

struct IconPair {
    esphome::image::Image *day  = nullptr;
    esphome::image::Image *night = nullptr;
};

inline std::unordered_map<std::string, IconPair> &icon_registry() {
    static std::unordered_map<std::string, IconPair> reg;
    return reg;
}

// Call this from YAML (on_setup) to wire IDs into C++
inline void register_icon(const std::string &state,
                          esphome::image::Image *day,
                          esphome::image::Image *night) {
    icon_registry()[state] = IconPair{day, night};
}

inline bool is_night_hour(int hour, int night_start = 21, int night_end = 6) {
    // Night if hour >= night_start OR hour < night_end (wraps midnight)
    return (hour >= night_start) || (hour < night_end);
}

inline void draw_weather_image(esphome::display::Display &it,
                               int x, int y,
                               const std::string &state,
                               int this_hour,
                               int night_start = 21,
                               int night_end = 6) {
    auto itf = icon_registry().find(state);
    if (itf == icon_registry().end()) return;

    const bool night = is_night_hour(this_hour, night_start, night_end);
    esphome::image::Image *img = night ? itf->second.night : itf->second.day;
    if (!img) return;

    //   img->next_frame();                         // advance animation if applicable
    it.image(x, y, img, COLOR_ON, COLOR_OFF);  // draw
}

} // namespace ui
