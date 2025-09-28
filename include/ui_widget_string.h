#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include <iostream>

namespace ui {
    struct StringPostArgs {
        std::string value;
    };

    template <std::size_t BufSize>
    class StringWidget : public Widget {
    private:
        esphome::display::Display *it = nullptr;
        ui::Box prev_box{};
        esphome::display::TextAlign align = esphome::display::TextAlign::LEFT;
        esphome::font::Font *font = nullptr;
        esphome::Color font_color = esphome::Color::WHITE;
        esphome::Color blank_color = esphome::Color::BLACK;
        std::string fmt;
        // Remember last value
        std::optional<std::string> new_value{};
        std::optional<std::string> last{};

        char buf[BufSize];

        static const std::string default_fmt() {
            return std::string("%s");
        }

        // Format into buf and update last
        void prep(std::string value, const char* fmt) {
            std::snprintf(buf, sizeof(buf), fmt, value.c_str());
            last = value;
        }

        bool is_different(std::string value) const {
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
            const StringPostArgs* post_args_ptr =
                std::any_cast<const StringPostArgs>(&args.extras);

            if (post_args_ptr == nullptr) return;

            std::string value = post_args_ptr->value;
            new_value = value;
        }

        void update() {
            if (!initialized) return;
            if (new_value.has_value() && !is_different(*new_value)) return;
            prep(*new_value, fmt.c_str());
            blank();
            write();
        }
        const int width() {
            if (!initialized) return 0;
            int x1, y1, w, h;
            it->get_text_bounds(anchor.x, anchor.y, buf, font,
                                align, &x1, &y1, &w, &h);
            return w;
        }
    };
}
