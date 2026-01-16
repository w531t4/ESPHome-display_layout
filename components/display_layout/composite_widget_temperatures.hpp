// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "argsbag.hpp"
#include "base_widget_composite.hpp"
#include "base_widget_text_numeric.hpp"
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include <array>
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
        struct RowSpec {
            const char *name;
            esphome::Color color;
        };

        RowSpec k_rows[] = {
            {"high", RED},
            {"current", TEAL},
            {"low", BLUE},
        };

        constexpr int y_shift = 1;
        constexpr int float_bufsize = 4;
        for (size_t i = 0; i < std::size(k_rows); ++i) {
            members[i] =
                std::make_unique<NumericWidget<float, float_bufsize>>();
            members[i]->initialize(InitArgs{
                .it = a.it,
                .id = a.id + "[" + k_rows[i].name + "]",
                .anchor =
                    ui::Coord(a.anchor.x, a.anchor.y + y_shift + (11 * i)),
                .font = a.font,
                .font_color = k_rows[i].color,
                .fmt = std::string("%3.0f"),
                .extras =
                    ArgsBag::of(TextInitArgs<float>{.right_align = true})});
        }
        initialized = true;
    }

    // void run(std::span<const float> values) {
    void post(const PostArgs &args) override {
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