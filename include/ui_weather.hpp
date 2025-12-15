#pragma once
#include "esphome/components/image/image.h"
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

} // namespace ui
