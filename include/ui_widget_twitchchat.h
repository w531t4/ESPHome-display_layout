#pragma once
#include <iostream>
#include <array>
#include <span>
#include "esphome/core/time.h"
#include "ui_widget.h"
#include "ui_widgetcomposite.h"
#include "ui_widget_twitchstring.h"
#include "ui_shared.h"
#include "ui_colors.h"

namespace ui {
    struct TwitchChatPostArgs {
        std::string row1;
        std::string row2;
        std::string row3;
    };

    template <std::size_t BufSize>
    class TwitchChatWidget : public CompositeWidget<3> {
    public:
        void initialize(const InitArgs& a) override {
            CompositeWidget<3>::initialize(a);
            const esphome::Color font_color = YELLOW;
            members[0] = std::make_unique<TwitchStringWidget<BufSize>>();
            members[0]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(anchor.x, anchor.y),      .font = *a.font,
                                            .font_color = font_color});
            members[1] = std::make_unique<TwitchStringWidget<BufSize>>();
            members[1]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(anchor.x, anchor.y + 11), .font = *a.font,
                                            .font_color = font_color});
            members[2] = std::make_unique<TwitchStringWidget<BufSize>>();
            members[2]->initialize(InitArgs{.it = a.it, .anchor = ui::Coord(anchor.x, anchor.y + 21), .font = *a.font,
                                            .font_color = font_color});
            initialized = true;
        }
        void set_capacity(const std::size_t cap, const bool preserve = true) {
            for (auto &p : members) {
                if (!p) continue;
                // All three members are created as DynStringWidget<BufSize> in initialize()
                auto *row = static_cast<TwitchStringWidget<BufSize>*>(p.get());
                row->set_capacity(cap, preserve);
            }
        }
        void post(const PostArgs& args) {
            if (args.extras.has_value()) {
                const TwitchChatPostArgs *post_args_ptr = std::any_cast<const TwitchChatPostArgs>(&args.extras);
                if (post_args_ptr != nullptr) {
                    members[0]->post(PostArgs{.extras = ui::StringPostArgs{.value = post_args_ptr->row1}});
                    members[1]->post(PostArgs{.extras = ui::StringPostArgs{.value = post_args_ptr->row2}});
                    members[2]->post(PostArgs{.extras = ui::StringPostArgs{.value = post_args_ptr->row3}});
                }
            }
        }
    };
}
