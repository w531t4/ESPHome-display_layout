// MyDerived.hpp
#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include <iostream>

namespace ui {
    struct FloatRunArgs {
        float value = -1;
    };

    template <std::size_t BufSize>
    class FloatWidget : public Widget {
    private:
        esphome::display::Display *it;
        ui::Box prev_box;
        esphome::display::TextAlign align;
        esphome::font::Font *font;
        esphome::Color font_color;
        esphome::Color blank_color;
        float last;
        char buf[BufSize];
        int priority;

    public:
        // constexpr std::size_t size() const noexcept { return BufSize; }
        void initialize(const InitArgs& a) override {
            this->it = a.it;
            this->anchor = a.anchor;
            this->font = a.font;
            this->align = a.align.value_or(esphome::display::TextAlign::LEFT);
            this->font_color = a.font_color.value_or(esphome::Color::WHITE);
            this->blank_color = a.blank_color.value_or(esphome::Color::BLACK);
            last = -400.0;
            initialized = true;

            // if (a.extras.has_value()) {
            //     const FloatInit *float_init_ptr =
            //         std::any_cast<const FloatInit>(&a.extras);
            //     if (float_init_ptr != nullptr) {
            //         // char buf[float_init_ptr->bufsize];
            //         // use float_init_ptr->bufsize, float_init_ptr->precision, float_init_ptr->show_units ...
            //     }
            // }
        }
        // blank applicable space
        void blank() override {
            ui::mywipe(it, prev_box, blank_color);
        }

        const bool is_different(const float value) {
            return value != last;
        }

        void prep(const float value, const std::string fmtstring) {
            snprintf(buf, sizeof(buf), fmtstring.c_str(), value);
            last = value;
        }

        // write-out to display
        void write() override {
            ui::myprint(it, font, anchor.x, anchor.y, buf, align, font_color, prev_box);
        }

        void run(const RunArgs& args) {
            if (args.extras.has_value()) {
                const FloatRunArgs *run_args_ptr = std::any_cast<const FloatRunArgs>(&args.extras);
                if (run_args_ptr != nullptr) {
                    if (!is_different(run_args_ptr->value)) return;
                    prep(run_args_ptr->value, "%0.f");
                    blank();
                    write();
                }
            }
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