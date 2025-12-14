// MyDerived.hpp
#pragma once
#include "argsbag.hpp"
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget.hpp"
#include "ui_widget_text_numeric.hpp"
#include "ui_widgetcomposite.hpp"
#include <array>
#include <iostream>
#include <span>

namespace ui {
struct TemperaturePostArgs {
    std::span<const float> values{};
};

class TemperaturesWidget : public CompositeWidget<3> {
  public:
    // constexpr std::size_t size() const noexcept { return BufSize; }
    void initialize(const InitArgs &a) override {
        // std::array<std::unique_ptr<Widget>, 3> members;
        CompositeWidget<3>::initialize(a);
        constexpr int float_bufsize = 4;
        members[0] =
            std::make_unique<NumericWidget<float, float_bufsize>>(); // HIGH
        members[0]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[high]",
                     .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11 * 0)),
                     .font = a.font,
                     .font_color = RED,
                     .fmt = std::string("%3.0f"),
                     .extras = ArgsBag::of(TextInitArgs{.right_align = true})});
        members[1] =
            std::make_unique<NumericWidget<float, float_bufsize>>(); // CURRENT
        members[1]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[current]",
                     .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11 * 1)),
                     .font = a.font,
                     .font_color = TEAL,
                     .fmt = std::string("%3.0f"),
                     .extras = ArgsBag::of(TextInitArgs{.right_align = true})});
        members[2] =
            std::make_unique<NumericWidget<float, float_bufsize>>(); // LOW
        members[2]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[low]",
                     .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11 * 2)),
                     .font = a.font,
                     .font_color = BLUE,
                     .fmt = std::string("%3.0f"),
                     .extras = ArgsBag::of(TextInitArgs{.right_align = true})});
        initialized = true;
    }

    // void run(std::span<const float> values) {
    void post(const PostArgs &args) {
        if (args.extras.has_value()) {
            const TemperaturePostArgs *post_args_ptr =
                std::any_cast<const TemperaturePostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                const std::size_t n =
                    std::min(members.size(), post_args_ptr->values.size());
                for (std::size_t i = 0; i < n; i++) {
                    if (members[i]) {
                        members[i]->post(
                            PostArgs{.extras = ui::NumericPostArgs<float>{
                                         .value = post_args_ptr->values[i]}});
                    }
                }
            }
        }
    }
};
} // namespace ui