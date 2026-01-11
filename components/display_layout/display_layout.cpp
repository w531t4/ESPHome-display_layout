// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#include "display_layout.h"
#include "esphome/components/globals/globals_component.h"
#include "esphome/core/log.h"
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
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

namespace esphome {
namespace display_layout {

static const char *TAG = "display_layout.component";
static constexpr std::size_t kChatBufferSize = 96;

void DisplayLayout::setup() {}

void DisplayLayout::loop() {}

void DisplayLayout::set_right_edge_x(int px) { right_edge_x_ = px; }

void DisplayLayout::reset() {
    ESP_LOGI(TAG, "Resetting display layout state");
    registry_ = ui::WidgetRegistry<kMaxWidgets>{};
    widgets_.clear();
    motion_widgets_.clear();
    resource_map_.clear();
    built_ = false;
}
struct WidgetMeta {
    WidgetKind kind;
    const char *name;
    std::unique_ptr<Widget> (*factory)();
    void (*registrar)(ui::WidgetRegistry<DisplayLayout::kMaxWidgets> &,
                      Widget *);
    bool is_motion{false};
};

template <typename W, bool is_motion = false>
WidgetMeta make_meta(WidgetKind kind, const char *name) {
    return WidgetMeta{
        kind, name,
        []() -> std::unique_ptr<Widget> { return std::make_unique<W>(); },
        [](ui::WidgetRegistry<DisplayLayout::kMaxWidgets> &reg, Widget *w) {
            reg.add(*static_cast<W *>(w));
        },
        is_motion};
}

namespace {
const WidgetMeta kWidgetMeta[] = {
    make_meta<ui::TwitchStreamerIconsWidget>(WidgetKind::TWITCH_ICONS,
                                             "twitch_icons"),
    make_meta<ui::TwitchChatWidget<kChatBufferSize>>(WidgetKind::TWITCH_CHAT,
                                                     "twitch_chat"),
    make_meta<ui::PixelMotionWidget, true>(WidgetKind::PIXEL_MOTION,
                                           "pixel_motion"),
    make_meta<ui::NetworkTputWidget>(WidgetKind::NETWORK_TPUT, "network_tput"),
    make_meta<
        ui::WeatherWidget<ui::WeatherCachedPostArgs, ui::WeatherPostArgs>>(
        WidgetKind::WEATHER, "weather"),
    make_meta<ui::TemperaturesWidget>(WidgetKind::TEMPERATURES, "temperatures"),
    make_meta<ui::DateWidget>(WidgetKind::DATE, "date"),
    make_meta<ui::TimeWidget>(WidgetKind::TIME, "time"),
    make_meta<ui::HAUpdatesWidget>(WidgetKind::HA_UPDATES, "ha_updates"),
    make_meta<ui::PSNWidget>(WidgetKind::PSN, "psn"),
};

const WidgetMeta *meta_for_kind(WidgetKind kind) {
    for (const auto &meta : kWidgetMeta) {
        if (meta.kind == kind)
            return &meta;
    }
    return nullptr;
}
} // namespace

std::string DisplayLayout::kind_to_string(WidgetKind kind) const {
    auto *meta = meta_for_kind(kind);
    if (meta != nullptr)
        return meta->name;
    return "unknown";
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

std::unique_ptr<Widget> DisplayLayout::make_widget(const WidgetConfig &cfg) {
    auto *meta = meta_for_kind(cfg.kind);
    if (meta == nullptr)
        return nullptr;
    return meta->factory();
}

void DisplayLayout::register_widget(const WidgetConfig &cfg,
                                    std::unique_ptr<Widget> widget) {
    if (!widget)
        return;

    auto *meta = meta_for_kind(cfg.kind);
    if (meta == nullptr) {
        ESP_LOGW(TAG, "register_widget(): unexpected widget kind");
        return;
    }

    Widget *raw = widget.get();
    if (meta->registrar) {
        meta->registrar(registry_, raw);
    }

    if (!cfg.resource.empty()) {
        resource_map_[cfg.resource] = raw;
    }
    if (meta->is_motion) {
        motion_widgets_.push_back(raw);
    }

    widgets_.push_back(std::move(widget));
}

void DisplayLayout::register_callbacks(const WidgetConfig &cfg,
                                       Widget *widget) {
    switch (cfg.kind) {
#ifdef USE_SENSOR
    case WidgetKind::NETWORK_TPUT: {
        auto *rx = cfg.source_rx.value_or(nullptr);
        auto *tx = cfg.source_tx.value_or(nullptr);
        if (!rx || !tx || !widget)
            return;
        auto post_now = [widget, rx, tx]() {
            widget->post(PostArgs{.extras = ui::NetworkTputPostArgs{
                                      .rx = rx->state, .tx = tx->state}});
        };
        rx->add_on_state_callback([post_now](float) { post_now(); });
        tx->add_on_state_callback([post_now](float) { post_now(); });
        post_now();
        break;
    }
#endif
#ifdef USE_TEXT_SENSOR
    case WidgetKind::PSN: {
        auto *phil = cfg.source_psn_phil.value_or(nullptr);
        auto *nick = cfg.source_psn_nick.value_or(nullptr);
        if (!phil || !nick || !widget)
            return;
        auto post_now = [widget, phil, nick]() {
            widget->post(PostArgs{
                .extras = ui::PSNPostArgs{.phil = phil, .nick = nick}});
        };
        phil->add_on_state_callback([post_now](std::string) { post_now(); });
        nick->add_on_state_callback([post_now](std::string) { post_now(); });
        post_now();
        break;
    }
#endif
    default:
        break;
    }
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
            args.extras.set(ui::TwitchChatInitArgs{
                .pixels_per_character = *cfg.pixels_per_character});
        }
        if (cfg.kind == WidgetKind::TWITCH_ICONS && cfg.icon_width &&
            cfg.icon_height && cfg.max_icons) {
            args.extras.set(
                ui::TwitchStreamerIconsInitArgs{.icon_width = *cfg.icon_width,
                                                .icon_height = *cfg.icon_height,
                                                .max_icons = *cfg.max_icons});
        }

        widget->initialize(args);
        register_callbacks(cfg, widget.get());
        register_widget(cfg, std::move(widget));
    }
}

void DisplayLayout::render(esphome::display::Display &it) {
    if (!built_) {
        // Build widgets once
        build_widgets(it);
        built_ = true;
    }

    post_from_sources();

    // Allow widgets that expect continuous movement to advance.
    for (auto *widget : motion_widgets_) {
        if (widget) {
            widget->post(PostArgs{});
        }
    }

    registry_.update_all();
    registry_.relayout();
}

void DisplayLayout::post_from_sources() {
    const std::size_t count = std::min(widget_configs_.size(), widgets_.size());
    for (std::size_t i = 0; i < count; ++i) {
        auto &cfg = widget_configs_[i];
        auto *widget = widgets_[i].get();
        if (!widget)
            continue;
        if (cfg.kind == WidgetKind::NETWORK_TPUT)
            continue;
        if (cfg.kind == WidgetKind::PSN)
            continue;

        switch (cfg.kind) {
        case WidgetKind::TWITCH_ICONS: {
#ifdef USE_TEXT_SENSOR
            auto *image = cfg.source_image.value_or(nullptr);
            auto *count_sensor = cfg.source_count.value_or(nullptr);
            auto *ready_flag = cfg.source_ready_flag.value_or(nullptr);
            if (!image || !count_sensor || !ready_flag)
                break;
            if (!count_sensor->has_state() || count_sensor->state.empty())
                break;

            if (!globals::id(ready_flag))
                break;
            const int num_icons =
                ui::TwitchStreamerIconsWidget::normalize_input(
                    count_sensor->state.c_str());
            widget->post(PostArgs{.extras = ui::TwitchStreamerIconsPostArgs{
                                      .image = image, .num_icons = num_icons}});
            globals::id(ready_flag) = false;
#endif
            break;
        }
        case WidgetKind::TWITCH_CHAT: {
#ifdef USE_TEXT_SENSOR
            auto *row = cfg.source_chat_row.value_or(nullptr);
            auto *channel = cfg.source_chat_channel.value_or(nullptr);
            if (!row)
                break;

            if (channel && !ui::txt_sensor_has_healthy_state(channel)) {
                if (cfg.twitch_started) {
                    widget->blank();
                    cfg.twitch_started = false;
                }
                break;
            }
            if (!ui::txt_sensor_has_healthy_state(row)) {
                if (cfg.twitch_started) {
                    widget->blank();
                    cfg.twitch_started = false;
                }
                break;
            }
            static std::array<std::string, 3> chat_history = {"", "", ""};
            static bool seeded = false;
            if (!seeded) {
                chat_history[2] = row->state;
                seeded = true;
            }

            const std::string &incoming = row->state;
            if (!incoming.empty() && incoming != chat_history[2]) {
                chat_history[0] = chat_history[1];
                chat_history[1] = chat_history[2];
                chat_history[2] = incoming;
            }
            widget->post(PostArgs{
                .extras = ui::TwitchChatPtrPostArgs{.row1 = &chat_history[0],
                                                    .row2 = &chat_history[1],
                                                    .row3 = &chat_history[2]}});
            cfg.twitch_started = true;
#endif
            break;
        }
        case WidgetKind::WEATHER: {
#if defined(USE_TEXT_SENSOR) && defined(USE_TIME)
            auto *weather = cfg.source_weather.value_or(nullptr);
            auto *clock = cfg.source_time.value_or(nullptr);
            if (!weather || !clock)
                break;
            auto now = clock->now();
            widget->post(
                PostArgs{.extras = ui::WeatherPostArgs{.ptr = &weather->state,
                                                       .this_hour = now.hour}});
#endif
            break;
        }
        case WidgetKind::TEMPERATURES: {
#ifdef USE_SENSOR
            auto *high = cfg.source_temp_high.value_or(nullptr);
            auto *current = cfg.source_temp_now.value_or(nullptr);
            auto *low = cfg.source_temp_low.value_or(nullptr);
            if (!high || !current || !low)
                break;
            const float values[3] = {high->state, current->state, low->state};
            widget->post(
                PostArgs{.extras = ui::TemperaturePostArgs{
                             .values = std::span<const float>(values, 3)}});
#endif
            break;
        }
        case WidgetKind::DATE: {
#ifdef USE_TIME
            auto *clock = cfg.source_time.value_or(nullptr);
            if (!clock)
                break;
            auto now = clock->now();
            widget->post(
                PostArgs{.extras = ui::DatePostArgs{.day = now.day_of_month,
                                                    .month = now.month}});
#endif
            break;
        }
        case WidgetKind::TIME: {
#ifdef USE_TIME
            auto *clock = cfg.source_time.value_or(nullptr);
            if (!clock)
                break;
            auto now = clock->now();
            widget->post(
                PostArgs{.extras = ui::TimePostArgs{.hour = now.hour,
                                                    .minute = now.minute,
                                                    .second = now.second}});
#endif
            break;
        }
        case WidgetKind::HA_UPDATES: {
#ifdef USE_SENSOR
            auto *value = cfg.source_updates.value_or(nullptr);
            if (!value)
                break;
            widget->post(
                PostArgs{.extras = ui::HAUpdatesPostArgs{
                             .value = static_cast<int>(value->state)}});
#endif
            break;
        }
        case WidgetKind::PIXEL_MOTION:
        default:
            break;
        }
    }
}

bool DisplayLayout::post_to_resource(const std::string &resource,
                                     const PostArgs &args) {
    if (!built_)
        return true;
    return post_to_resource_internal(resource, args);
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

void DisplayLayout::dump_config() {
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

} // namespace display_layout
} // namespace esphome
