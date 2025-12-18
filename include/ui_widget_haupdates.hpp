#pragma once
#include "argsbag.hpp"
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget_text_numeric.hpp"
#include "ui_widgetcomposite.hpp"
#include <array>

namespace ui {
struct HAUpdatesPostArgs {
    int value;
};
class HAUpdatesWidget : public CompositeWidget<1> {
  public:
    void initialize(const InitArgs &a) override {
        CompositeWidget<1>::initialize(a);
        constexpr int bufsize = 3; // "1000 TX\0"
        members[0] = std::make_unique<NumericWidget<int, bufsize>>(); // HIGH
        members[0]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[0]",
                     .anchor = ui::Coord(a.anchor.x, a.anchor.y),
                     .font = a.font,
                     .font_color = RED,
                     .fmt = std::string("%d"),
                     .extras = ArgsBag::of(TextInitArgs<int>{
                         .right_align = true, .hide_if_equal_val = 0})});
        initialized = true;
    }

    void post(const PostArgs &args) override {
        if (args.extras.has_value()) {
            const HAUpdatesPostArgs *post_args_ptr =
                std::any_cast<const HAUpdatesPostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                members[0]->post(PostArgs{.extras = ui::NumericPostArgs<int>{
                                              .value = post_args_ptr->value}});
            }
        }
    }
};
} // namespace ui
