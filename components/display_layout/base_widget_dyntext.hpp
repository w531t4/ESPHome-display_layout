// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "base_widget.hpp"
#include "base_widget_text.hpp"
#include "ui_capabilities.hpp"
#include "ui_shared.hpp"
#include <algorithm>
#include <vector>

namespace ui {
// struct TextInitArgs {
//     std::optional<bool> right_align;
//     std::optional<char> max_width_padding_char;
//     std::optional<int> trim_pixels_top;         // Use top/bottom to remove
//     pixels from top/bottom of characters for certain fonts std::optional<int>
//     trim_pixels_bottom;
// };
template <typename T, typename P, std::size_t BufSize>
class DynTextWidget : public Widget, public ui::IBufferResizable {
  private:
    static constexpr const char *TAG = "ui_widget_dyntext";

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
    std::optional<T> new_value{};
    std::optional<T> last{};

    std::vector<char> buf;

    // Pick a default printf format based on T
    virtual constexpr const std::string default_fmt() = 0;

    virtual void prep(T value, const char *fmt) = 0;

    virtual bool is_different(P value) const = 0;

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
        }
        this->last.reset();
        this->new_value.reset();
        buf.assign(std::max<std::size_t>(BufSize ? BufSize : 16, 2),
                   '\0'); // dynamic buffer, at least 2 bytes
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
    void horizontal_shift(const int pixels) override {
        Widget::horizontal_shift(pixels);
        this->prev_box = ui::Box{this->prev_box.x1 + pixels, this->prev_box.y1,
                                 this->prev_box.w, this->prev_box.h};
    }

    void write() override {
        const int y = anchor.y - trim_pixels_top;
        if (right_align) {
            // printf will start drawing at the first pixel of a character,
            // ignoring leading whitespace in buffer.
            const int curr_buf_width = bounds(buf.data()).w;
            const int x_draw = anchor.x + (this->width() - curr_buf_width);
            ui::myprint(it, font, x_draw, y, buf.data(), align, font_color,
                        prev_box);
        } else {
            ui::myprint(it, font, anchor.x, y, buf.data(), align, font_color,
                        prev_box);
        }
    }

    // must store value in this->last
    virtual void copy_value(P value) = 0;

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

    void update() {
        if (!initialized)
            return;
        if (!this->is_dirty())
            return;
        if (!this->last.has_value())
            return;
        // if (!new_value.has_value())
        //     return;
        // if (new_value.has_value() && !is_different(*new_value))
        //     return;
        prep(*this->last, fmt.c_str());
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
        std::vector<char> tmp(buf.size(), padding_value);
        if (!tmp.empty())
            tmp.back() = '\0';
        return bounds(tmp.data()).w;
        ;
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
        return bounds(buf.data()).h - trim_pixels_top - trim_pixels_bottom;
    }

    // Set buffer capacity at runtime (chars incl. '\0').
    // If preserve=true, keep existing contents (truncated if shrinking).
    // If preserve=false, clear/fill with NULs.
    void set_capacity(std::size_t cap, bool preserve = true) override {
        ESP_LOGD(TAG, "[widget=%s] set_capacity: before buf=%s",
                 this->get_name().c_str(), this->buf.data());
        cap = std::max<std::size_t>(cap, 2); // always leave room for '\0'
        if (preserve) {
            buf.resize(cap);
            if (!buf.empty())
                buf.back() = '\0'; // keep valid C-string
        } else {
            buf.assign(cap, '\0'); // zero-fill
        }
        ESP_LOGD(TAG, "[widget=%s] set_capacity: after buf=%s",
                 this->get_name().c_str(), this->buf.data());
        this->prev_box = ui::Box{this->prev_box.x1, this->prev_box.y1,
                                 this->width(), this->prev_box.h};
    }

    const size_t get_capacity() const { return this->buf.size(); }

    std::size_t capacity() const { return buf.size(); }

    std::size_t length() const {
        // current C-string length within capacity
        return buf.empty() ? 0 : strnlen(buf.data(), buf.size());
    }
};
} // namespace ui
