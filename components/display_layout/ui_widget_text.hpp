// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "ui_shared.hpp"
#include "ui_widget.hpp"
#include <algorithm>

namespace ui {
template <typename T> struct TextInitArgs {
    std::optional<bool> right_align;
    std::optional<char> max_width_padding_char;
    std::optional<int>
        trim_pixels_top; // Use top/bottom to remove pixels from top/bottom of
                         // characters for certain fonts
    std::optional<int> trim_pixels_bottom;
    std::optional<T> hide_if_equal_val;
};
template <typename T, typename P, std::size_t BufSize>
class TextWidget : public Widget {
  private:
    static constexpr const char *TAG = "ui_widget_text";

  protected:
    ui::Box prev_box{};
    esphome::display::TextAlign align = esphome::display::TextAlign::LEFT;
    esphome::font::Font *font = nullptr;
    esphome::Color font_color = esphome::Color::WHITE;
    esphome::Color blank_color = esphome::Color::BLACK;
    std::string fmt;
    bool right_align = false;
    char max_width_padding_char = '8';
    // Remember last value
    uint8_t trim_pixels_top = 0;
    uint8_t trim_pixels_bottom = 0;
    std::optional<T> hide_if_equal_val;
    std::optional<T> new_value{};
    std::optional<T> last{};

    char buf[BufSize];

    // Pick a default printf format based on T
    virtual constexpr const std::string default_fmt() const = 0;

    virtual void prep(T value, const char *fmt) = 0;

    virtual bool is_different(P value) const = 0;

    // must store value in this->last
    virtual void copy_value(P value) = 0;

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
        this->font = *a.font;
        this->align = a.align.value_or(esphome::display::TextAlign::LEFT);
        this->font_color = a.font_color.value_or(esphome::Color::WHITE);
        this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);
        this->fmt = a.fmt.value_or(this->default_fmt());

        if (auto *t = a.extras.get<TextInitArgs<T>>()) {
            if (t->max_width_padding_char.has_value())
                this->max_width_padding_char = *t->max_width_padding_char;
            if (t->trim_pixels_top.has_value())
                this->trim_pixels_top = *t->trim_pixels_top;
            if (t->trim_pixels_bottom.has_value())
                this->trim_pixels_bottom = *t->trim_pixels_bottom;
            if (t->right_align.has_value())
                this->right_align = *t->right_align;
            if (t->hide_if_equal_val.has_value())
                this->hide_if_equal_val = *t->hide_if_equal_val;
        }
        this->last.reset();
        this->new_value.reset();
        buf[0] = '\0';
        initialized = true;
    }

    void blank() override {
        if (trim_pixels_top > 0 || trim_pixels_bottom > 0) {
            // we clip here because prev_box isn't updated correctly from
            // myprint()
            //                                                         this + 1
            //                                                         feels
            //                                                         like a
            //                                                         hack.
            it->start_clipping(anchor.x, anchor.y, anchor.x + this->width() + 1,
                               anchor.y + height());
        }
        ui::mywipe(it, prev_box, blank_color);
        if (trim_pixels_top > 0 || trim_pixels_bottom > 0) {
            it->end_clipping();
        }
    }

    void write() override {
        const int y = anchor.y - trim_pixels_top;
        if (right_align) {
            // printf will start drawing at the first pixel of a character,
            // ignoring leading whitespace in buffer.
            const int curr_buf_width = bounds(buf).w;
            const int x_draw = anchor.x + (this->width() - curr_buf_width);
            ui::myprint(it, font, x_draw, y, buf, align, font_color, prev_box);
        } else {
            ui::myprint(it, font, anchor.x, y, buf, align, font_color,
                        prev_box);
        }
    }

    void post(const PostArgs &args) override {
        if (!initialized)
            return;
        const P *post_args_ptr = std::any_cast<const P>(&args.extras);

        if (post_args_ptr == nullptr)
            return;
        if (!is_different(*post_args_ptr))
            return;
        this->copy_value(*post_args_ptr);
        this->set_dirty(true);
    }

    void update() override {
        if (!initialized)
            return;
        if (!this->last.has_value())
            return;
        if (this->hide_if_equal_val.has_value()) {
            if (this->hide_if_equal_val.value() == this->last.value()) {
                if (this->is_visible()) {
                    this->blank();
                    this->set_visible(false);
                }
                return;
            } else if (!(this->is_visible())) {
                this->set_visible(true);
            }
        }
        if (!this->is_dirty())
            return;
        prep(this->last.value(), fmt.c_str());

        blank();
        write();
        this->set_dirty(false);
    }

    const ui::Box bounds(const char *buffer) const {
        int x1, y1, w, h;
        it->get_text_bounds(anchor.x, anchor.y, buffer, font, align, &x1, &y1,
                            &w, &h);
        return ui::Box{x1, y1, w, h};
    }

    const int get_max_width(const char padding_value) const {
        char fullwidth_buf[BufSize];
        std::fill_n(fullwidth_buf, BufSize - 1, padding_value);
        fullwidth_buf[BufSize - 1] = '\0';
        return bounds(fullwidth_buf).w;
    }

    const int width() const override {
        if (!initialized)
            return 0;
        if (!(this->is_visible()))
            return 0;
        return get_max_width(max_width_padding_char);
    }

    const int height() const override {
        if (!initialized)
            return 0;
        return bounds(buf).h - trim_pixels_top - trim_pixels_bottom;
    }
};
} // namespace ui
