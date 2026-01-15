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
    std::optional<int> last{};
    /// the value immediately preceeding last
    int prev;
    /// forward: true (downward), false (upward)
    bool forward;

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
        *this->last = 16;
        prev = *(this->last) - 1;
        this->forward = true;
        initialized = true;
    }

    void blank() override {
        ui::Box box =
            ui::Box{anchor.x, anchor.y, this->width(), this->height()};
        ui::mywipe(it, box, blank_color);
    }

    void write() override { it->draw_pixel_at(anchor.x, *this->last, GREEN); }

    void action() {
        prev = *this->last;
        // forward = 'downward'
        if (forward && *this->last < (height() - 1)) {
            // if moving down to bottom
            (*this->last)++;
        } else if (forward && *this->last == (height() - 1)) {
            // if moving down and have reached the bottom
            (*this->last)--;
            forward = false;
        } else if (*this->last > anchor.y) {
            // if moving upwards to top
            (*this->last)--;
        } else if (*this->last == anchor.y) {
            // if moving up and have reached the top
            (*this->last)++;
            forward = true;
        }
    }
    void post(const PostArgs &args) override {
        if (!initialized)
            return;
        this->action();
        this->set_dirty(true);
    }

    // void post() { post(PostArgs{}); }

    void update() override {
        if (!initialized)
            return;
        if (!this->is_dirty())
            return;
        // prep(*new_value, fmt.c_str());
        blank();
        write();
        this->set_dirty(false);
    }

    const int width() const override {
        if (!(this->is_visible()))
            return 0;
        return 1;
    }

    const int height() const override { return 32; }
};
} // namespace ui
