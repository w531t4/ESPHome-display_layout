#pragma once
#include "argsbag.hpp"
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget_text_numeric.hpp"
#include "ui_widgetcomposite.hpp"
#include <array>

namespace ui {
struct NetworkTputPostArgs {
    float rx;
    float tx;
};
class NetworkTputWidget : public CompositeWidget<2> {
  public:
    void initialize(const InitArgs &a) override {
        CompositeWidget<2>::initialize(a);
        constexpr int float_bufsize = 8; // "1000 TX\0"
        members[0] =
            std::make_unique<NumericWidget<float, float_bufsize>>(); // HIGH
        members[0]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[tx]",
                     .anchor = ui::Coord(a.anchor.x, a.anchor.y),
                     .font = a.font,
                     .font_color = RED,
                     .fmt = std::string("%4.0f TX"),
                     .extras = ArgsBag::of(TextInitArgs{.right_align = true})});
        members[1] =
            std::make_unique<NumericWidget<float, float_bufsize>>(); // CURRENT
        members[1]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[rx]",
                     .anchor = ui::Coord(a.anchor.x, a.anchor.y + 11),
                     .font = a.font,
                     .font_color = TEAL,
                     .fmt = std::string("%4.0f RX"),
                     .extras = ArgsBag::of(TextInitArgs{.right_align = true})});
        initialized = true;
    }

    void post(const PostArgs &args) {
        if (args.extras.has_value()) {
            const NetworkTputPostArgs *post_args_ptr =
                std::any_cast<const NetworkTputPostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                members[0]->post(PostArgs{.extras = ui::NumericPostArgs<float>{
                                              .value = post_args_ptr->tx}});
                members[1]->post(PostArgs{.extras = ui::NumericPostArgs<float>{
                                              .value = post_args_ptr->rx}});
            }
        }
    }
};
} // namespace ui
