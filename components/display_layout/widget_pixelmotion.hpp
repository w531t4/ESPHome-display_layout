// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "base_widget.hpp"
#include "ui_colors.hpp"
#include "ui_shared.hpp"

namespace ui {
class PixelMotionWidget : public Widget {
  protected:
    ui::Box prev_box{};
    esphome::Color blank_color = esphome::Color::BLACK;

    std::optional<int> new_value{};
    std::optional<int> last{};
    bool forward = true;

    bool is_different(const int value) const {
        if (!last.has_value())
            return true;
        return value != last;
    }

  public:
    void initialize(const InitArgs &a) override {
        Widget::initialize(a);
        this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);

        this->last.reset();
        this->new_value.reset();
        *this->new_value = 16;
        *this->last = *this->new_value;
        initialized = true;
    }

    void blank() override { it->draw_pixel_at(anchor.x, *last, blank_color); }

    void write() override { it->draw_pixel_at(anchor.x, *new_value, GREEN); }

    void action() {
        // forward = 'downward'
        if (forward && *new_value < (height() - 1)) {
            // if moving down to bottom
            (*new_value)++;
        } else if (forward && *new_value == (height() - 1)) {
            // if moving down and have reached the bottom
            (*new_value)--;
            forward = false;
        } else if (*new_value > anchor.y) {
            // if moving upwards to top
            (*new_value)--;
        } else if (*new_value == anchor.y) {
            // if moving up and have reached the top
            (*new_value)++;
            forward = true;
        }
    }
    void post(const PostArgs &args) override {
        if (!initialized)
            return;
        this->action();
    }

    // void post() { post(PostArgs{}); }

    void update() override {
        if (!initialized)
            return;
        if (new_value.has_value() && !is_different(*new_value))
            return;
        // prep(*new_value, fmt.c_str());
        blank();
        write();
        last = *new_value;
    }

    const int width() const override {
        if (!(this->is_visible()))
            return 0;
        return 1;
    }

    const int height() const override { return 32; }
};
} // namespace ui
