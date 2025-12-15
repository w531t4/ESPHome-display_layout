#pragma once
#include "argsbag.hpp"
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget_text_numeric.hpp"
#include "ui_widget_text_string.hpp"
#include "ui_widgetcomposite.hpp"
#include <array>

namespace ui {
struct TimePostArgs {
    int hour;
    int minute;
    int second;
};

class TimeWidget : public CompositeWidget<4> {
  public:
    void initialize(const InitArgs &a) override {
        CompositeWidget<4>::initialize(a);
        if (!a.font2.has_value()) {
            ESP_LOGE("widget_time", "Required font2 parameter missing\n");
            return;
        }
        if (!*a.font2) {
            ESP_LOGE("widget_time", "Required font2 must not be nullptr\n");
            return;
        }
        members[0] = std::make_unique<NumericWidget<int, 3>>(); // HOURS
        members[0]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[hours]",
                     .anchor = anchor,
                     .font = a.font,
                     .font_color = ORANGE,
                     .fmt = std::string("%02d"),
                     .extras = ArgsBag::of(TextInitArgs{
                         .trim_pixels_top = 6, .trim_pixels_bottom = 6})});
        members[1] = std::make_unique<StringWidget<2>>(); // COLON
        members[1]->initialize(InitArgs{
            .it = a.it,
            .id = a.id + "[colon]",
            .anchor = ui::Coord(anchor.x + 27, anchor.y),
            .font = a.font, // 33
            .font_color = ORANGE,
            .extras = ArgsBag::of(TextInitArgs{.right_align = true,
                                               .trim_pixels_top = 6,
                                               .trim_pixels_bottom = 6})});
        members[2] = std::make_unique<NumericWidget<int, 3>>(); // MINUTES
        members[2]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[min]",
                     .anchor = ui::Coord(anchor.x + 45, anchor.y),
                     .font = a.font,
                     .font_color = ORANGE,
                     .fmt = std::string("%02d"),
                     .extras = ArgsBag::of(TextInitArgs{
                         .trim_pixels_top = 6, .trim_pixels_bottom = 6})});
        members[3] = std::make_unique<NumericWidget<int, 3>>(); // SECONDS
        members[3]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[sec]",
                     .anchor = ui::Coord(anchor.x + 87, anchor.y),
                     .font = *a.font2,
                     .font_color = ORANGE,
                     .fmt = std::string("%02d")});
        initialized = true;
    }

    void post(const PostArgs &args) {
        if (args.extras.has_value()) {
            const TimePostArgs *post_args_ptr =
                std::any_cast<const TimePostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                members[0]->post(PostArgs{.extras = ui::NumericPostArgs<int>{
                                              .value = post_args_ptr->hour}});
                members[1]->post(PostArgs{
                    .extras = ui::StringPostArgs{.value = std::string(":")}});
                members[2]->post(PostArgs{.extras = ui::NumericPostArgs<int>{
                                              .value = post_args_ptr->minute}});
                members[3]->post(PostArgs{.extras = ui::NumericPostArgs<int>{
                                              .value = post_args_ptr->second}});
            }
        }
    }
};
} // namespace ui