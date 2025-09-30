#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include <iostream>
#include <algorithm>

namespace ui {
    template <typename T>
    struct NumericPostArgs {
        T value;
    };

    template <typename T, std::size_t BufSize>
    class NumericWidget : public Widget {
    private:
        esphome::display::Display *it = nullptr;
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
        static constexpr const std::string default_fmt() {
            if constexpr (std::is_integral<T>::value) {
                return std::string("%d");
            } else {
                return std::string("%0.f");
            }
        }

        // Format into buf and update last
        void prep(T value, const char* fmt) {
            if constexpr (std::is_integral<T>::value) {
                //std::snprintf(buf, sizeof(buf), fmt, static_cast<long long>(value));
                std::snprintf(buf, sizeof(buf), fmt, value);
            } else {
                std::snprintf(buf, sizeof(buf), fmt, static_cast<double>(value));
            }
            last = value;
        }

        bool is_different(T value) const {
            if (!last.has_value()) return true;
            return value != last.value();
        }

    public:
        void initialize(const InitArgs& a) override {
            this->it          = a.it;
            this->anchor      = a.anchor;
            this->font        = a.font;
            this->align       = a.align.value_or(esphome::display::TextAlign::LEFT);
            this->font_color  = a.font_color.value_or(esphome::Color::WHITE);
            this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);
            this->fmt         = a.fmt.value_or(this->default_fmt());

            this->use_max_width_as_width = a.use_max_width_as_width.value_or(true);
            if (a.max_width_padding_char.has_value())
                this->max_width_padding_char = *a.max_width_padding_char;

            this->priority    = a.priority;
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

        struct NumericInit {
            const char* fmt = default_fmt();
        };

        void post(const PostArgs& args) override {
            if (!initialized) return;
            const NumericPostArgs<T>* post_args_ptr =
                std::any_cast<const NumericPostArgs<T>>(&args.extras);

            if (post_args_ptr == nullptr) return;

            T value = post_args_ptr->value;
            new_value = value;
        }

        void update() {
            if (!initialized) return;
            if (new_value.has_value() && !is_different(*new_value)) return;

            // const char* fmt = default_fmt();
            // if (args.extras.has_value()) {
            //     const NumericInit* ni = std::any_cast<const NumericInit>(&args.extras);
            //     if (ni != nullptr && ni->fmt != nullptr) fmt = ni->fmt;
            // }

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
