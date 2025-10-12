#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include "ui_widget_text.h"
#include <iostream>
#include <algorithm>
#include <vector>

namespace ui {
    struct IBufferResizable {
        virtual void set_capacity(std::size_t cap, bool preserve = true) = 0;
    };
    // struct TextInitArgs {
    //     std::optional<bool> right_align;
    //     std::optional<bool> use_max_width_as_width;
    //     std::optional<char> max_width_padding_char;
    //     std::optional<int> trim_pixels_top;         // Use top/bottom to remove pixels from top/bottom of characters for certain fonts
    //     std::optional<int> trim_pixels_bottom;
    // };
    template <typename T, typename P, std::size_t BufSize>
    class DynTextWidget : public Widget, public IBufferResizable {
    protected:
        ui::Box prev_box{};
        esphome::display::TextAlign align = esphome::display::TextAlign::LEFT;
        esphome::font::Font *font = nullptr;
        esphome::Color font_color = esphome::Color::WHITE;
        esphome::Color blank_color = esphome::Color::BLACK;
        std::string fmt;
        bool right_align = false;
        bool use_max_width_as_width = true;
        char max_width_padding_char = '8';
        // Remember last value
        uint8_t trim_pixels_top = 0;
        uint8_t trim_pixels_bottom = 0;
        std::optional<T> new_value{};
        std::optional<T> last{};

        std::vector<char> buf;
        int max_width = -1;

        // Pick a default printf format based on T
        virtual constexpr const std::string default_fmt() = 0;

        virtual void prep(T value, const char *fmt) = 0;

        bool is_different(T value) const {
            if (!last.has_value()) return true;
            return value != last.value();
        }

    public:
        void initialize(const InitArgs& a) override {
            Widget::initialize(a);
            if (!a.font.has_value()) {
                ESP_LOGE("text_widget","Required font parameter missing\n");
                return;
            }
            this->font        = *a.font;
            this->align       = a.align.value_or(esphome::display::TextAlign::LEFT);
            this->font_color  = a.font_color.value_or(esphome::Color::WHITE);
            this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);
            this->fmt         = a.fmt.value_or(this->default_fmt());

            if (auto* t = a.extras.get<TextInitArgs>()) {
                this->use_max_width_as_width = t->use_max_width_as_width.value_or(this->use_max_width_as_width);
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
            buf.assign(std::max<std::size_t>(BufSize ? BufSize : 16, 2), '\0');  // dynamic buffer, at least 2 bytes
            initialized = true;
        }

        void blank() override {
            if (trim_pixels_top > 0 || trim_pixels_bottom > 0) {
                // we clip here because prev_box isn't updated correctly from myprint()
                //                                                         this + 1 feels like a hack.
                it->start_clipping(anchor.x, anchor.y, anchor.x + max_width + 1, anchor.y + height());
            }
            ui::mywipe(it, prev_box, blank_color);
            if (trim_pixels_top > 0 || trim_pixels_bottom > 0) {
                it->end_clipping();
            }
        }

        void write() override {
            const int y = anchor.y - trim_pixels_top;
            if (use_max_width_as_width && right_align) {
                // printf will start drawing at the first pixel of a character, ignoring leading
                // whitespace in buffer.
                const int curr_buf_width = bounds(buf.data()).w;
                const int x_draw = anchor.x + (max_width - curr_buf_width);
                ui::myprint(it, font, x_draw, y, buf.data(), align, font_color, prev_box);
            } else {
                ui::myprint(it, font, anchor.x, y, buf.data(), align, font_color, prev_box);
            }
        }

        void post(const PostArgs& args) override {
            if (!initialized) return;
            const P* post_args_ptr =
                std::any_cast<const P>(&args.extras);

            if (post_args_ptr == nullptr) return;

            T value = post_args_ptr->value;
            new_value = value;
        }

        void update() {
            if (!initialized) return;
            if (new_value.has_value() && !is_different(*new_value)) return;
            prep(*new_value, fmt.c_str());
            blank();
            write();
        }

        const ui::Box bounds(const char* buffer) {
            int x1, y1, w, h;
            it->get_text_bounds(anchor.x, anchor.y, buffer, font,
                                align, &x1, &y1, &w, &h);
            return ui::Box{x1, y1, w, h};
        }

        const int get_max_width(const char padding_value) {
            if (max_width >= 0) return max_width;
            std::vector<char> tmp(buf.size(), padding_value);
            if (!tmp.empty()) tmp.back() = '\0';
            max_width = bounds(tmp.data()).w;
            return max_width;
        }

        const int width() {
            if (!initialized) return 0;
            if (use_max_width_as_width) return get_max_width(max_width_padding_char);
            return bounds(buf.data()).w;
        }

        const int height() {
            if (!initialized) return 0;
            return bounds(buf.data()).h - trim_pixels_top - trim_pixels_bottom;
        }

        // Set buffer capacity at runtime (chars incl. '\0').
        // If preserve=true, keep existing contents (truncated if shrinking).
        // If preserve=false, clear/fill with NULs.
        void set_capacity(std::size_t cap, bool preserve = true) {
            cap = std::max<std::size_t>(cap, 2);        // always leave room for '\0'
            if (preserve) {
                buf.resize(cap);
                if (!buf.empty()) buf.back() = '\0';    // keep valid C-string
            } else {
                buf.assign(cap, '\0');                  // zero-fill
            }
            max_width = -1;                             // invalidate cached slot width
        }

        std::size_t capacity() const { return buf.size(); }

        std::size_t length() const {
            // current C-string length within capacity
            return buf.empty() ? 0 : strnlen(buf.data(), buf.size());
        }

    };
}
