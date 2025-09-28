#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include <iostream>

namespace ui {
    template <typename T>
    struct NumericRunArgs {
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
        std::optional<T> last{};

        char buf[BufSize];

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

            this->last.reset();
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

        void run(const RunArgs& args) override {
            if (!initialized) return;
            const NumericRunArgs<T>* run_args_ptr =
                std::any_cast<const NumericRunArgs<T>>(&args.extras);

            if (run_args_ptr == nullptr) return;

            T value = run_args_ptr->value;
            if (!is_different(value)) return;

            // const char* fmt = default_fmt();
            // if (args.extras.has_value()) {
            //     const NumericInit* ni = std::any_cast<const NumericInit>(&args.extras);
            //     if (ni != nullptr && ni->fmt != nullptr) fmt = ni->fmt;
            // }

            prep(value, fmt.c_str());
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
