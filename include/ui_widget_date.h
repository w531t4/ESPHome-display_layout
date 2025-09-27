// MyDerived.hpp
#pragma once
#include <iostream>
#include <array>
#include <span>
#include "esphome/core/time.h"
#include "ui_widget.h"
#include "ui_widgetcomposite.h"
#include "ui_widget_numeric.h"
#include "ui_shared.h"
#include "ui_colors.h"

namespace ui {
    struct DateRunArgs {
        uint8_t day;
        uint8_t month;
    };

    class DateWidget : public CompositeWidget<2> {
    public:
        inline static constexpr const char *months[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        void initialize(const InitArgs& a) override {
            // std::array<std::unique_ptr<Widget>, 3> members;
            CompositeWidget<2>::initialize(a);
            members[0] = std::make_unique<NumericWidget<uint8_t,3>>(); // DAY
            members[0]->initialize(InitArgs{.it = a.it, .anchor = a.anchor, .font = a.font, .font_color = PURPLE, .fmt = std::string("%02d")});
            // members[1] = std::make_unique<NumericWidget<float,float_bufsize>>(); // CURRENT
            // members[1]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11*1)), .font = a.font, .font_color = TEAL});
            // members[2] = std::make_unique<NumericWidget<float,float_bufsize>>(); // LOW
            // members[2]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(a.anchor.x, a.anchor.y + (11*2)), .font = a.font, .font_color = BLUE});
            initialized = true;
        }

        void run(const RunArgs& args) {
            if (args.extras.has_value()) {
                const DateRunArgs *run_args_ptr = std::any_cast<const DateRunArgs>(&args.extras);
                if (run_args_ptr != nullptr) {
                    members[0]->run(RunArgs{.extras = ui::NumericRunArgs<uint8_t>{.value = run_args_ptr->day}});
                    // const std::size_t n = std::min(members.size(), run_args_ptr->values.size());
                    // for (std::size_t i = 0; i < n; i++) {
                    //     if (members[i]) {
                    //         members[i]->run(RunArgs{.extras = ui::NumericRunArgs<int>{.value = run_args_ptr->values[i]}});
                    //     }
                    // }
                }
            }
        }
    };
}