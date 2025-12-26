
// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
#include "esphome/core/component.h"
#include "magnet.hpp"
#include "ui_shared.hpp"
#include "ui_widget.hpp"
#include "ui_widgetregistry.hpp"
#ifndef DISPLAY_LAYOUT_MAX_WIDGETS
#define DISPLAY_LAYOUT_MAX_WIDGETS 16
#endif
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace esphome {
namespace homeassistant {
class HomeassistantTextSensor;
} // namespace homeassistant
namespace sensor {
class Sensor;
} // namespace sensor
namespace text_sensor {
class TextSensor;
} // namespace text_sensor
namespace time {
class RealTimeClock;
} // namespace time
namespace globals {
template <typename T> class GlobalsComponent;
template <typename T> T &id(GlobalsComponent<T> *value);
} // namespace globals
namespace display_layout {

enum class WidgetKind {
    TWITCH_ICONS,
    TWITCH_CHAT,
    PIXEL_MOTION,
    NETWORK_TPUT,
    WEATHER,
    TEMPERATURES,
    DATE,
    TIME,
    HA_UPDATES,
    PSN
};

struct WidgetConfig {
    WidgetKind kind{WidgetKind::PIXEL_MOTION};
    std::string id;
    std::string resource;
    ui::Coord anchor{0, 0};
    uint8_t priority{0};
    Magnet magnet{Magnet::RIGHT};
    std::optional<esphome::font::Font *> font;
    std::optional<esphome::font::Font *> font2;
    std::optional<int> pixels_per_character;
    std::optional<int> icon_width;
    std::optional<int> icon_height;
    std::optional<int> max_icons;
    std::optional<esphome::display::BaseImage *> source_image;
    std::optional<esphome::text_sensor::TextSensor *> source_count;
    std::optional<esphome::globals::GlobalsComponent<bool> *> source_ready_flag;
    std::optional<esphome::homeassistant::HomeassistantTextSensor *>
        source_chat_row1;
    std::optional<esphome::homeassistant::HomeassistantTextSensor *>
        source_chat_row2;
    std::optional<esphome::homeassistant::HomeassistantTextSensor *>
        source_chat_row3;
    std::optional<esphome::homeassistant::HomeassistantTextSensor *>
        source_chat_channel;
    std::optional<esphome::sensor::Sensor *> source_rx;
    std::optional<esphome::sensor::Sensor *> source_tx;
    std::optional<esphome::text_sensor::TextSensor *> source_weather;
    std::optional<esphome::time::RealTimeClock *> source_time;
    std::optional<esphome::sensor::Sensor *> source_temp_high;
    std::optional<esphome::sensor::Sensor *> source_temp_now;
    std::optional<esphome::sensor::Sensor *> source_temp_low;
    std::optional<esphome::sensor::Sensor *> source_updates;
    std::optional<esphome::homeassistant::HomeassistantTextSensor *>
        source_psn_phil;
    std::optional<esphome::homeassistant::HomeassistantTextSensor *>
        source_psn_nick;
    bool twitch_started{false};
};

class DisplayLayout : public Component {
  public:
    static constexpr std::size_t kMaxWidgets = DISPLAY_LAYOUT_MAX_WIDGETS;
    void setup() override;
    void loop() override;
    void dump_config() override;

    // Build widgets on first render, then update/layout every frame.
    void add_widget_config(const WidgetConfig &cfg);
    void set_gap_x(int px) { gap_x_ = px; }
    void set_right_edge_x(int px);
    void render(esphome::display::Display &it);
    // Post by resource name so callers don't keep widget handles.
    bool post_to_resource(const std::string &resource, const PostArgs &args);

  private:
    std::string kind_to_string(WidgetKind kind) const;
    void build_widgets(esphome::display::Display &it);
    void configure_registry();
    std::unique_ptr<Widget> make_widget(const WidgetConfig &cfg);
    void register_widget(const WidgetConfig &cfg,
                         std::unique_ptr<Widget> widget);
    Widget *widget_for_resource(const std::string &resource);
    void post_from_sources();
    bool post_to_resource_internal(const std::string &resource,
                                   const PostArgs &args);

    std::vector<WidgetConfig> widget_configs_;
    std::vector<std::unique_ptr<Widget>> widgets_;
    ui::WidgetRegistry<kMaxWidgets> registry_;
    std::unordered_map<std::string, Widget *> resource_map_;
    // Widgets that need a tick each frame (e.g. PixelMotion).
    std::vector<Widget *> motion_widgets_;
    bool built_ = false;
    int gap_x_ = 0;
    std::optional<int> right_edge_x_;
};

} // namespace display_layout
} // namespace esphome
