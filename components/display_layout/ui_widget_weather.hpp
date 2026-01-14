// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "esphome/components/display/display.h"
#include "ui_shared.hpp"
#include "ui_weather.hpp"
#include "ui_widget.hpp"

namespace ui {
struct WeatherCachedPostArgs {
    std::string value;
    int this_hour;
};
struct WeatherPostArgs {
    const std::string *ptr;
    int this_hour;
};

template <typename T, typename P> class WeatherWidget : public Widget {
  private:
    static constexpr const char *TAG = "ui_widget_weather";

  protected:
    ui::Box prev_box{};
    esphome::Color blank_color = esphome::Color::BLACK;
    // Remember last value
    std::optional<T> last{};

    int night_start = 21;
    int night_end = 6;
    // Pick a default printf format based on T

    bool is_different(P value) const {
        if (!last.has_value())
            return true;
        return (*value.ptr != last->value) ||
               (value.this_hour != last->this_hour);
    }

  public:
    void initialize(const InitArgs &a) override {
        Widget::initialize(a);
        if (!a.font.has_value()) {
            ESP_LOGE(
                TAG,
                "[widget=%s] initialize(): Required font parameter missing",
                this->get_name().c_str());
            return;
        }
        this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);

        this->last.reset();
        initialized = true;
    }

    void blank() override { ui::mywipe(it, prev_box, blank_color); }

    void write() override {
        if (!last.has_value())
            return;
        auto itf = icon_registry().find(last->value);
        if (itf == icon_registry().end())
            return;
        esphome::image::Image *img =
            ui::is_night_hour(last->this_hour, night_start, night_end)
                ? itf->second.night
                : itf->second.day;
        if (!img)
            return;
        it->image(anchor.x, anchor.y, img, esphome::display::COLOR_ON,
                  esphome::display::COLOR_OFF); // draw
        prev_box = {anchor.x, anchor.y, width(), width()};
    }

    void post(const PostArgs &args) override {
        if (!initialized)
            return;
        const P *post_args_ptr = std::any_cast<const P>(&args.extras);

        if (post_args_ptr == nullptr)
            return;

        if (post_args_ptr->ptr == nullptr)
            return;

        if (!is_different(*post_args_ptr))
            return;
        last = WeatherCachedPostArgs{.value = *post_args_ptr->ptr,
                                     .this_hour = post_args_ptr->this_hour};

        this->set_dirty(true);
    }

    void update() override {
        if (!initialized)
            return;
        if (!this->is_dirty())
            return;
        blank();
        write();
        this->set_dirty(false);
    }

    const int width() const override {
        if (!(this->is_visible()))
            return 0;
        return 32;
    }

    const int height() const override { return 32; }
};
} // namespace ui
