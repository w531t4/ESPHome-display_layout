// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "argsbag.hpp"
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget_text_numeric.hpp"
#include "ui_widget_text_string.hpp"
#include "base_widget_composite.hpp"
#include <array>

namespace ui {
struct DatePostArgs {
    uint8_t day;
    uint8_t month;
};

class DateWidget : public CompositeWidget<2> {
  private:
    static constexpr const char *TAG = "ui_widget_date";

  public:
    inline static constexpr const std::string months[13] = {
        std::string("JAN"), std::string("FEB"), std::string("MAR"),
        std::string("APR"), std::string("MAY"), std::string("JUN"),
        std::string("JUL"), std::string("AUG"), std::string("SEP"),
        std::string("OCT"), std::string("NOV"), std::string("DEC"),
        std::string("UNK")};
    void initialize(const InitArgs &a) override {
        // std::array<std::unique_ptr<Widget>, 3> members;
        CompositeWidget<2>::initialize(a);
        if (!a.font2.has_value()) {
            ESP_LOGE(
                TAG,
                "[widget=%s] initialize(): Required font2 parameter missing",
                this->get_name().c_str());
            return;
        }
        if (!*a.font2) {
            ESP_LOGE(TAG,
                     "[widget=%s] initialize(): Required font2 must not "
                     "be nullptr",
                     this->get_name().c_str());
            return;
        }
        members[0] = std::make_unique<NumericWidget<uint8_t, 3>>(); // DAY
        members[0]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[day]",
                     .anchor = ui::Coord(anchor.x, anchor.y + 10),
                     .font = a.font,
                     .font_color = PURPLE,
                     .fmt = std::string("%02d"),
                     .extras = ArgsBag::of(TextInitArgs<uint8_t>{
                         .trim_pixels_top = 9, .trim_pixels_bottom = 9})});
        members[1] = std::make_unique<StringWidget<4>>(); // MONTH
        members[1]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[month]",
                     .anchor = ui::Coord(anchor.x + 10 + 2, anchor.y),
                     .font = *a.font2,
                     .font_color = PINK});
        // members[1] = std::make_unique<NumericWidget<float,float_bufsize>>();
        // // CURRENT members[1]->initialize(InitArgs{.it = a.it, .anchor =
        // ui::Coord(a.anchor.x, a.anchor.y + (11*1)), .font = a.font,
        // .font_color = TEAL}); members[2] =
        // std::make_unique<NumericWidget<float,float_bufsize>>(); // LOW
        // members[2]->initialize(InitArgs{.it = a.it, .anchor =
        // ui::Coord(a.anchor.x, a.anchor.y + (11*2)), .font = a.font,
        // .font_color = BLUE});
        initialized = true;
    }
    const std::string *get_month(const uint8_t month) {
        if (month < 1 || month > 12)
            return &months[12]; // UNK
        return &months[month - 1];
    }
    void post(const PostArgs &args) override {
        if (args.extras.has_value()) {
            const DatePostArgs *post_args_ptr =
                std::any_cast<const DatePostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                members[0]->post(
                    PostArgs{.extras = ui::NumericPostArgs<uint8_t>{
                                 .value = post_args_ptr->day}});
                members[1]->post(
                    PostArgs{.extras = ui::StringPtrPostArgs{
                                 .ptr = get_month(post_args_ptr->month)}});
                // const std::size_t n = std::min(members.size(),
                // post_args_ptr->values.size()); for (std::size_t i = 0; i < n;
                // i++) {
                //     if (members[i]) {
                //         members[i]->run(PostArgs{.extras =
                //         ui::NumericPostArgs<int>{.value =
                //         post_args_ptr->values[i]}});
                //     }
                // }
            }
        }
    }
};
} // namespace ui