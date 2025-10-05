#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include "ui_weather.h"
#include <iostream>
#include <algorithm>

namespace ui {
    class PixelMotionWidget : public Widget {
    protected:
        ui::Box prev_box{};
        esphome::Color blank_color = esphome::Color::BLACK;

        std::optional<int> new_value{};
        std::optional<int> last{};
        bool mvright = true;

        bool is_different(const int value) const {
            if (!last.has_value()) return true;
            return value != last;
        }

    public:
        void initialize(const InitArgs& a) override {
            Widget::initialize(a);
            this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);

            this->last.reset();
            this->new_value.reset();
            *this->new_value = 16;
            *this->last = *this->new_value;
            initialized = true;
        }

        void blank() override {
            it->draw_pixel_at(anchor.x, *last, blank_color);
        }

        void write() override {
            it->draw_pixel_at(anchor.x, *new_value, GREEN);
        }

        void post(const PostArgs& args) override {
            if (!initialized) return;
            if (mvright && *new_value < (height()-1)) {
                (*new_value)++;
            } else if (mvright && *new_value == (height()-1)) {
                (*new_value)--;
                mvright = false;
            } else if (*new_value > anchor.y) {
                (*new_value)--;
            } else if (*new_value == anchor.y) {
                (*new_value)++;
                mvright = true;
            }
        }

        void post() {
            post(PostArgs{});
        }

        void update() {
            if (!initialized) return;
            if (new_value.has_value() && !is_different(*new_value)) return;
            // prep(*new_value, fmt.c_str());
            blank();
            write();
            last = *new_value;
        }

        const int width() {
            return 1;
        }

        const int height() {
            return 32;
        }
    };
}
