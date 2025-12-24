
// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
#include "magnet.hpp"
#include "ui_shared.hpp"
#include "ui_widget.hpp"
#include "ui_widgetregistry.hpp"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace esphome {
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
};

class DisplayLayout : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

   // Build widgets on first render, then update/layout every frame.
   void add_widget_config(const WidgetConfig &cfg);
   void set_gap_x(int px) { gap_x_ = px; }
   void set_right_edge_x(int px);
   void render(esphome::display::Display &it);
   // Post/blank by resource name so callers don't keep widget handles.
   bool post_to_resource(const std::string &resource, const PostArgs &args);
   bool blank_resource(const std::string &resource);

  private:
   struct PendingPost {
     std::string resource;
     PostArgs args;
   };

   static constexpr std::size_t kMaxWidgets = 16;
   std::string kind_to_string(WidgetKind kind) const;
   void build_widgets(esphome::display::Display &it);
   void configure_registry();
   std::unique_ptr<Widget> make_widget(const WidgetConfig &cfg);
   void register_widget(const WidgetConfig &cfg,
                        std::unique_ptr<Widget> widget);
   Widget *widget_for_resource(const std::string &resource);
   bool post_to_resource_internal(const std::string &resource,
                                  const PostArgs &args);
   bool blank_resource_internal(const std::string &resource);

   std::vector<WidgetConfig> widget_configs_;
   std::vector<std::unique_ptr<Widget>> widgets_;
   ui::WidgetRegistry<kMaxWidgets> registry_;
   std::unordered_map<std::string, Widget *> resource_map_;
   // Widgets that need a tick each frame (e.g. PixelMotion).
   std::vector<Widget *> motion_widgets_;
   std::vector<PendingPost> pending_posts_;
   std::vector<std::string> pending_blanks_;
   bool built_ = false;
   int gap_x_ = 0;
   std::optional<int> right_edge_x_;
};


}  // namespace display_layout
}  // namespace esphome
