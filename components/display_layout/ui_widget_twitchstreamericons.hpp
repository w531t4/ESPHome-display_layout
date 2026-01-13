// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "esphome/components/display/display.h"
#include "esphome/components/image/image.h"
#include "ui_shared.hpp"
#include "ui_widget.hpp"

namespace ui {
struct TwitchStreamerIconsPostArgs {
    esphome::image::Image *image;
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

    int icon_width, icon_height, max_icons;
    int prev_num_icons;

    bool is_different(TwitchStreamerIconsPostArgs value) const {
        if (!last.has_value())
            return true;
        // TODO: This isn't sufficient. count(Before)=4 could = count(After)=4
        // but set(Before) could != set(After)
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
        this->prev_num_icons = 0;
        initialized = true;
    }

    void blank() override {
        if (!last.has_value())
            return;
        if (this->prev_num_icons > last->num_icons) {
            it->filled_rectangle(anchor.x, anchor.y,
                                 this->prev_num_icons * this->icon_width,
                                 this->height(), this->blank_color);
        }
    }

    void write() override {
        if (!last.has_value())
            return;
        if (!last->image)
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
        it->image(anchor.x, anchor.y, last->image); // draw
        it->end_clipping();
        // TODO: Why is the following line here?
        it->image(anchor.x, anchor.y, last->image, esphome::display::COLOR_ON,
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

        ESP_LOGI(TAG, "[widget=%s] post(): new_num_icons=%d",
                 this->get_name().c_str(), post_args_ptr->num_icons);
        if (!is_different(*post_args_ptr))
            return;
        if (last.has_value())
            this->prev_num_icons = last->num_icons;
        last = *post_args_ptr;
        this->set_dirty(true);
    }

    void update() override {
        if (!initialized)
            return;
        if (!this->is_dirty())
            return;
        // before we change our size, wipe out what we're currently using
        blank();
        write();
        this->set_dirty(false);
    }

    const int width() const override {
        if (!initialized)
            return -1;
        if (!(this->is_visible()))
            return 0;
        if (!last.has_value())
            return 0;
        return icon_width * last->num_icons;
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
