#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include <iostream>
#include <algorithm>

namespace ui {
    template <typename T, typename P, std::size_t BufSize>
    class TextWidget : public Widget {
    protected:
        ui::Box prev_box{};
        esphome::display::TextAlign align = esphome::display::TextAlign::LEFT;
        esphome::font::Font *font = nullptr;
        esphome::Color font_color = esphome::Color::WHITE;
        esphome::Color blank_color = esphome::Color::BLACK;
        std::string fmt;
        // Remember last value
        std::optional<T> new_value{};
        std::optional<T> last{};

        char buf[BufSize];
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
            this->font        = a.font;
            this->align       = a.align.value_or(esphome::display::TextAlign::LEFT);
            this->font_color  = a.font_color.value_or(esphome::Color::WHITE);
            this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);
            this->fmt         = a.fmt.value_or(this->default_fmt());

            this->use_max_width_as_width = a.use_max_width_as_width.value_or(true);
            if (a.max_width_padding_char.has_value())
                this->max_width_padding_char = *a.max_width_padding_char;

            this->last.reset();
            this->new_value.reset();
            buf[0] = '\0';
            initialized = true;
        }

        void blank() override {
            ui::mywipe(it, prev_box, blank_color);
        }

        void write() override {
            ui::myprint(it, font, anchor.x, anchor.y, buf, align, font_color, prev_box);
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

        const int width(const char* buffer) {
            if (!initialized) return 0;
            int x1, y1, w, h;
            it->get_text_bounds(anchor.x, anchor.y, buffer, font,
                                align, &x1, &y1, &w, &h);
            return w;
        }

        const int get_max_width(const char padding_value) {
            if (max_width >= 0) return max_width;
            char fullwidth_buf[BufSize];
            std::fill_n(fullwidth_buf, BufSize - 1, padding_value);
            fullwidth_buf[BufSize - 1] = '\0';
            max_width = width(fullwidth_buf);
            return max_width;
        }

        const int width() {
            if (use_max_width_as_width) return get_max_width(max_width_padding_char);
            return width(buf);
        }
    };
}
