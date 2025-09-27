// MyDerived.hpp
#pragma once
#include "ui_widget.h"
#include "ui_shared.h"
#include <iostream>

namespace ui {
    template <std::size_t BufSize>
    class FloatWidget : public Widget {
    private:
        esphome::display::Display *it;
        ui::Box prev_box;
        ui::Coord anchor{-1, -1};
        esphome::display::TextAlign align;
        esphome::font::Font *font;
        esphome::Color font_color;
        esphome::Color blank_color;
        float last;
        char buf[BufSize];
        int priority;

    public:
        // constexpr std::size_t size() const noexcept { return BufSize; }
        void initialize(esphome::display::Display &it,
                        const ui::Coord anchor,
                        const esphome::Color blank_color,
                        esphome::font::Font *font,
                        esphome::display::TextAlign align,
                        esphome::Color font_color) {
            this->it = &it;
            this->anchor = anchor;
            this->align = align;
            this->font = font;
            this->font_color = font_color;
            this->blank_color = blank_color;
            last = -400.0;

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

        void run(const float value) {
            if (!is_different(value)) return;
            prep(value, "%0.f");
            blank();
            write();
        }
        // void execute() override {
        //     std::cout << "Executing main logic\n";
        // }

        // void cleanup() override {
        //     std::cout << "Cleaning up resources\n";
        // }
    };
}