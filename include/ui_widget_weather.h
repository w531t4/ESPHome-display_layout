#pragma once
#include "ui_shared.h"
#include "ui_weather.h"
#include "ui_widget.h"
#include <algorithm>
#include <iostream>

namespace ui {
struct WeatherPostArgs {
    std::string value;
    int this_hour;
};

template <typename T, typename P> class WeatherWidget : public Widget {
  protected:
    ui::Box prev_box{};
    esphome::Color blank_color = esphome::Color::BLACK;
    // Remember last value
    std::optional<T> new_value{};
    std::optional<T> last{};

    esphome::image::Image *img = nullptr;
    int night_start = 21;
    int night_end = 6;
    // Pick a default printf format based on T

    bool is_different(T value) const {
        if (!last.has_value())
            return true;
        return (value.value != last->value) ||
               (value.this_hour != last->this_hour);
    }

  public:
    void initialize(const InitArgs &a) override {
        Widget::initialize(a);
        if (!a.font.has_value()) {
            ESP_LOGE("text_widget", "Required font parameter missing\n");
            return;
        }
        this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);

        this->last.reset();
        this->new_value.reset();

        initialized = true;
    }

    void blank() override { ui::mywipe(it, prev_box, blank_color); }

    void write() override {
        if (!img)
            return;
        it->image(anchor.x, anchor.y, img, COLOR_ON, COLOR_OFF); // draw
        prev_box = {anchor.x, anchor.y, width(), width()};
    }

    void post(const PostArgs &args) override {
        if (!initialized)
            return;
        const P *post_args_ptr = std::any_cast<const P>(&args.extras);

        if (post_args_ptr == nullptr)
            return;

        T value = *post_args_ptr;
        new_value = value;
    }

    void update() {
        if (!initialized)
            return;
        if (new_value.has_value() && !is_different(*new_value))
            return;
        last = *new_value;
        auto itf = icon_registry().find(new_value->value);
        if (itf == icon_registry().end())
            return;
        img = ui::is_night_hour(new_value->this_hour, night_start, night_end)
                  ? itf->second.night
                  : itf->second.day;
        if (!img)
            return;
        blank();
        write();
    }

    const int width() { return 32; }

    const int height() { return 32; }
};
} // namespace ui
