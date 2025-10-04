// MyDerived.hpp
#pragma once
#include <iostream>
#include <array>
#include <span>
#include "ui_widget.h"
#include "ui_widgetcomposite.h"
#include "ui_widget_text_numeric.h"
#include "ui_shared.h"
#include "ui_colors.h"

namespace ui {
    struct TemperaturePostArgs {
        std::span<const float> values{};
    };

    class TemperaturesWidget : public CompositeWidget<3> {
    public:
        // constexpr std::size_t size() const noexcept { return BufSize; }
        void initialize(const InitArgs& a) override {
            // std::array<std::unique_ptr<Widget>, 3> members;
            CompositeWidget<3>::initialize(a);
            constexpr int float_bufsize = 4;
            members[0] = std::make_unique<NumericWidget<float,float_bufsize>>(); // HIGH
            members[0]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11*0)),
                                            .font = a.font, .font_color = RED,  .fmt = std::string("%3.0f")});
            members[1] = std::make_unique<NumericWidget<float,float_bufsize>>(); // CURRENT
            members[1]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11*1)),
                                            .font = a.font, .font_color = TEAL, .fmt = std::string("%3.0f")});
            members[2] = std::make_unique<NumericWidget<float,float_bufsize>>(); // LOW
            members[2]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11*2)),
                                            .font = a.font, .font_color = BLUE, .fmt = std::string("%3.0f")});
            initialized = true;
        }

        // void run(std::span<const float> values) {
        void post(const PostArgs& args) {
            if (args.extras.has_value()) {
                const TemperaturePostArgs *post_args_ptr = std::any_cast<const TemperaturePostArgs>(&args.extras);
                if (post_args_ptr != nullptr) {
                    const std::size_t n = std::min(members.size(), post_args_ptr->values.size());
                    for (std::size_t i = 0; i < n; i++) {
                        if (members[i]) {
                            members[i]->post(PostArgs{.extras = ui::NumericPostArgs<float>{.value = post_args_ptr->values[i]}});
                        }
                    }
                }
            }
        }
    };
}