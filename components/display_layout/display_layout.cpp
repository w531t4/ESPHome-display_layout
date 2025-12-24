// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#include "esphome/core/log.h"
#include "display_layout.h"
#include "ui_widget_date.hpp"
#include "ui_widget_haupdates.hpp"
#include "ui_widget_network_tput.hpp"
#include "ui_widget_pixelmotion.hpp"
#include "ui_widget_psn.hpp"
#include "ui_widget_temperatures.hpp"
#include "ui_widget_time.hpp"
#include "ui_widget_twitchchat.hpp"
#include "ui_widget_twitchstreamericons.hpp"
#include "ui_widget_weather.hpp"

namespace esphome {
namespace display_layout {

static const char *TAG = "display_layout.component";
static constexpr std::size_t kChatBufferSize = 96;

void DisplayLayout::setup() {

}

void DisplayLayout::loop() {

}

void DisplayLayout::set_right_edge_x(int px) { right_edge_x_ = px; }

std::string DisplayLayout::kind_to_string(WidgetKind kind) const {
    switch (kind) {
    case WidgetKind::TWITCH_ICONS:
        return "twitch_icons";
    case WidgetKind::TWITCH_CHAT:
        return "twitch_chat";
    case WidgetKind::PIXEL_MOTION:
        return "pixel_motion";
    case WidgetKind::NETWORK_TPUT:
        return "network_tput";
    case WidgetKind::WEATHER:
        return "weather";
    case WidgetKind::TEMPERATURES:
        return "temperatures";
    case WidgetKind::DATE:
        return "date";
    case WidgetKind::TIME:
        return "time";
    case WidgetKind::HA_UPDATES:
        return "ha_updates";
    case WidgetKind::PSN:
        return "psn";
    default:
        return "unknown";
    }
}

void DisplayLayout::configure_registry() {
    registry_.set_gap_x(gap_x_);
    if (right_edge_x_.has_value()) {
        registry_.set_right_edge_x(*right_edge_x_);
    }
}

void DisplayLayout::add_widget_config(const WidgetConfig &cfg) {
    if (widget_configs_.size() >= kMaxWidgets) {
        ESP_LOGW(TAG,
                 "add_widget_config(): max widgets (%d) reached, ignoring %s",
                 static_cast<int>(kMaxWidgets), cfg.id.c_str());
        return;
    }
    widget_configs_.push_back(cfg);
}

std::unique_ptr<Widget> DisplayLayout::make_widget(
    const WidgetConfig &cfg) {
    switch (cfg.kind) {
    case WidgetKind::TWITCH_ICONS:
        return std::make_unique<ui::TwitchStreamerIconsWidget>();
    case WidgetKind::TWITCH_CHAT:
        return std::make_unique<ui::TwitchChatWidget<kChatBufferSize>>();
    case WidgetKind::PIXEL_MOTION:
        return std::make_unique<ui::PixelMotionWidget>();
    case WidgetKind::NETWORK_TPUT:
        return std::make_unique<ui::NetworkTputWidget>();
    case WidgetKind::WEATHER:
        return std::make_unique<
            ui::WeatherWidget<ui::WeatherPostArgs, ui::WeatherPostArgs>>();
    case WidgetKind::TEMPERATURES:
        return std::make_unique<ui::TemperaturesWidget>();
    case WidgetKind::DATE:
        return std::make_unique<ui::DateWidget>();
    case WidgetKind::TIME:
        return std::make_unique<ui::TimeWidget>();
    case WidgetKind::HA_UPDATES:
        return std::make_unique<ui::HAUpdatesWidget>();
    case WidgetKind::PSN:
        return std::make_unique<ui::PSNWidget>();
    default:
        return nullptr;
    }
}

void DisplayLayout::register_widget(const WidgetConfig &cfg,
                                    std::unique_ptr<Widget> widget) {
    if (!widget)
        return;

    Widget *raw = widget.get();
    switch (cfg.kind) {
    case WidgetKind::TWITCH_ICONS:
        registry_.add(
            *static_cast<ui::TwitchStreamerIconsWidget *>(raw));
        break;
    case WidgetKind::TWITCH_CHAT:
        registry_.add(*static_cast<ui::TwitchChatWidget<kChatBufferSize> *>(
            raw));
        break;
    case WidgetKind::PIXEL_MOTION:
        registry_.add(*static_cast<ui::PixelMotionWidget *>(raw));
        break;
    case WidgetKind::NETWORK_TPUT:
        registry_.add(*static_cast<ui::NetworkTputWidget *>(raw));
        break;
    case WidgetKind::WEATHER:
        registry_.add(*static_cast<
                      ui::WeatherWidget<ui::WeatherPostArgs,
                                        ui::WeatherPostArgs> *>(raw));
        break;
    case WidgetKind::TEMPERATURES:
        registry_.add(*static_cast<ui::TemperaturesWidget *>(raw));
        break;
    case WidgetKind::DATE:
        registry_.add(*static_cast<ui::DateWidget *>(raw));
        break;
    case WidgetKind::TIME:
        registry_.add(*static_cast<ui::TimeWidget *>(raw));
        break;
    case WidgetKind::HA_UPDATES:
        registry_.add(*static_cast<ui::HAUpdatesWidget *>(raw));
        break;
    case WidgetKind::PSN:
        registry_.add(*static_cast<ui::PSNWidget *>(raw));
        break;
    default:
        ESP_LOGW(TAG, "register_widget(): unexpected widget kind");
        break;
    }

    if (!cfg.resource.empty()) {
        resource_map_[cfg.resource] = raw;
    }
    if (cfg.kind == WidgetKind::PIXEL_MOTION) {
        motion_widgets_.push_back(raw);
    }

    widgets_.push_back(std::move(widget));
}

Widget *DisplayLayout::widget_for_resource(const std::string &resource) {
    auto it = resource_map_.find(resource);
    if (it == resource_map_.end())
        return nullptr;
    return it->second;
}

void DisplayLayout::build_widgets(esphome::display::Display &it) {
    configure_registry();

    for (const auto &cfg : widget_configs_) {
        auto widget = make_widget(cfg);
        if (!widget) {
            ESP_LOGW(TAG, "build_widgets(): skipping %s (unknown kind)",
                     cfg.id.c_str());
            continue;
        }
        InitArgs args{.it = &it,
                      .id = cfg.id,
                      .anchor = cfg.anchor,
                      .priority = cfg.priority,
                      .magnet = cfg.magnet};
        if (cfg.font.has_value())
            args.font = *cfg.font;
        if (cfg.font2.has_value())
            args.font2 = *cfg.font2;
        if (cfg.pixels_per_character.has_value()) {
            args.extras.set(
                ui::TwitchChatInitArgs{
                    .pixels_per_character = *cfg.pixels_per_character});
        }
        if (cfg.kind == WidgetKind::TWITCH_ICONS && cfg.icon_width &&
            cfg.icon_height && cfg.max_icons) {
            args.extras.set(
                ui::TwitchStreamerIconsInitArgs{
                    .icon_width = *cfg.icon_width,
                    .icon_height = *cfg.icon_height,
                    .max_icons = *cfg.max_icons});
        }

        widget->initialize(args);
        register_widget(cfg, std::move(widget));
    }
}

void DisplayLayout::render(esphome::display::Display &it) {
    if (!built_) {
        // Mirror the old per-lambda init: build widgets once, then replay any
        // queued posts/blanks that arrived before the first render.
        build_widgets(it);
        built_ = true;
        for (const auto &resource : pending_blanks_) {
            blank_resource_internal(resource);
        }
        pending_blanks_.clear();
        for (const auto &pending : pending_posts_) {
            post_to_resource_internal(pending.resource, pending.args);
        }
        pending_posts_.clear();
    }

    // Allow widgets that expect continuous movement to advance.
    for (auto *widget : motion_widgets_) {
        if (widget) {
            widget->post(PostArgs{});
        }
    }

    registry_.update_all();
    registry_.relayout();
}

bool DisplayLayout::post_to_resource(const std::string &resource,
                                     const PostArgs &args) {
    if (!built_) {
        pending_posts_.push_back(PendingPost{resource, args});
        return true;
    }
    return post_to_resource_internal(resource, args);
}

bool DisplayLayout::blank_resource(const std::string &resource) {
    if (!built_) {
        pending_blanks_.push_back(resource);
        return true;
    }
    return blank_resource_internal(resource);
}

bool DisplayLayout::post_to_resource_internal(const std::string &resource,
                                              const PostArgs &args) {
    auto *widget = widget_for_resource(resource);
    if (!widget) {
        ESP_LOGW(TAG, "post_to_resource(): no widget bound to resource '%s'",
                 resource.c_str());
        return false;
    }
    widget->post(args);
    return true;
}

bool DisplayLayout::blank_resource_internal(const std::string &resource) {
    auto *widget = widget_for_resource(resource);
    if (!widget) {
        ESP_LOGW(TAG, "blank_resource(): no widget bound to resource '%s'",
                 resource.c_str());
        return false;
    }
    if (widget->is_enabled() && widget->is_visible()) {
        widget->blank();
    }
    return true;
}

void DisplayLayout::dump_config(){
    ESP_LOGCONFIG(TAG, "Display Layout");
    ESP_LOGCONFIG(TAG, "  gap_x: %d", gap_x_);
    if (right_edge_x_.has_value()) {
        ESP_LOGCONFIG(TAG, "  right_edge_x: %d", *right_edge_x_);
    }
    for (const auto &cfg : widget_configs_) {
        ESP_LOGCONFIG(TAG,
                      "  widget id=%s type=%s anchor=(%d,%d) priority=%d "
                      "resource=%s magnet=%d",
                      cfg.id.c_str(), this->kind_to_string(cfg.kind).c_str(),
                      cfg.anchor.x, cfg.anchor.y, cfg.priority,
                      cfg.resource.c_str(), static_cast<int>(cfg.magnet));
    }
}


}  // namespace display_layout
}  // namespace esphome
