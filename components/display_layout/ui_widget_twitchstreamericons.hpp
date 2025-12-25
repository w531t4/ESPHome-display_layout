// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "esphome/components/display/display.h"
#include "ui_shared.hpp"
#include "ui_widget.hpp"

namespace ui {
struct TwitchStreamerIconsPostArgs {
    esphome::display::BaseImage *image;
    int num_icons;
};
struct TwitchStreamerIconsInitArgs {
    int icon_width;
    int icon_height;
    int max_icons;
};

class TwitchStreamerIconsWidget : public Widget {
  private:
    static constexpr const char *TAG = "ui_widget_twitchstreamericons";

  protected:
    ui::Box prev_box{};
    esphome::Color blank_color = esphome::Color::BLACK;
    // Remember last value
    std::optional<TwitchStreamerIconsPostArgs> new_value{};
    std::optional<TwitchStreamerIconsPostArgs> last{};

    esphome::display::BaseImage *img = nullptr;
    int icon_width, icon_height, max_icons;

    bool is_different(TwitchStreamerIconsPostArgs value) const {
        if (!last.has_value())
            return true;
        return (value.num_icons != last->num_icons);
    }

  public:
    void initialize(const InitArgs &a) override {
        Widget::initialize(a);
        this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);
        if (auto *t = a.extras.get<TwitchStreamerIconsInitArgs>()) {
            this->icon_width = t->icon_width;
            this->icon_height = t->icon_height;
            this->max_icons = t->max_icons;
        }
        this->last.reset();
        this->new_value.reset();

        initialized = true;
    }

    void blank() override {
        if ((*last).num_icons > (*new_value).num_icons) {
            it->filled_rectangle(anchor.x, anchor.y, this->width(),
                                 this->height(), this->blank_color);
        }
    }

    void write() override {
        if (!img)
            return;
        ESP_LOGI(
            TAG,
            "[widget=%s] write(): start_clipping: anchor.x=%d, anchor.y=%d, "
            "right=%d, "
            "bottom=%d, width=%d, height=%d",
            this->get_name().c_str(), anchor.x, anchor.y, this->width() - 1,
            this->height() - 1, this->width(), this->height());
        // clip so only the populated portion of twitch_strip is written.
        it->start_clipping(anchor.x, anchor.y, this->width() - 1,
                           this->height() - 1);
        it->image(anchor.x, anchor.y, this->img); // draw
        it->end_clipping();
        it->image(anchor.x, anchor.y, img, esphome::display::COLOR_ON,
                  esphome::display::COLOR_OFF); // draw
        prev_box = {anchor.x, anchor.y, width(), width()};
    }

    void post(const PostArgs &args) override {
        if (!initialized)
            return;
        const TwitchStreamerIconsPostArgs *post_args_ptr =
            std::any_cast<const TwitchStreamerIconsPostArgs>(&args.extras);

        if (post_args_ptr == nullptr)
            return;

        TwitchStreamerIconsPostArgs value = *post_args_ptr;
        ESP_LOGI(TAG, "[widget=%s] post(): new_num_icons=%d",
                 this->get_name().c_str(), value.num_icons);
        new_value = value;
    }

    void update() override {
        if (!initialized)
            return;
        if (!new_value.has_value())
            return;
        if (new_value.has_value() && !is_different(*new_value))
            return;
        // before we change our size, wipe out what we're currently using
        blank();
        last = *new_value;
        img = new_value.value().image;
        if (!(img))
            return;
        write();
    }

    const int width() const override {
        if (!initialized)
            return -1;
        if (!(this->is_visible()))
            return 0;
        return icon_width * (*last).num_icons;
    }

    const int height() const override {
        if (!initialized)
            return -1;
        return icon_height;
    }

    static inline int normalize_input(const char *data) {
        return static_cast<int>(std::atof(data));
    }
};
} // namespace ui
