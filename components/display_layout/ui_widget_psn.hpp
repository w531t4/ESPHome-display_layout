// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget.hpp"
#include "ui_widget_text_string.hpp"
#include "ui_widgetcomposite.hpp"
#include <array>

namespace ui {
struct PSNPostArgs {
    esphome::homeassistant::HomeassistantTextSensor *phil;
    esphome::homeassistant::HomeassistantTextSensor *nick;
};

class PSNWidget : public CompositeWidget<2> {
  public:
    void initialize(const InitArgs &a) override {
        CompositeWidget<2>::initialize(a);
        members[0] = std::make_unique<StringWidget<2>>(); // Phil
        members[0]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[phil]",
                     .anchor = ui::Coord(anchor.x, anchor.y),
                     .font = *a.font,
                     .font_color = GREEN,
                     .extras = ArgsBag::of(TextInitArgs<std::string>{
                         .right_align = true,
                         .hide_if_equal_val = std::string("unknown")})});
        members[1] = std::make_unique<StringWidget<2>>(); // Nick
        members[1]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[nick]",
                     .anchor = ui::Coord(anchor.x, anchor.y + 20),
                     .font = *a.font,
                     .font_color = PINK,
                     .extras = ArgsBag::of(TextInitArgs<std::string>{
                         .right_align = true,
                         .hide_if_equal_val = std::string("unknown")})});

        initialized = true;
    }
    void post(const PostArgs &args) override {
        if (args.extras.has_value()) {
            const PSNPostArgs *post_args_ptr =
                std::any_cast<const PSNPostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                if (post_args_ptr->phil->has_state()) {
                    members[0]->post(
                        PostArgs{.extras = ui::StringPtrPostArgs{
                                     .ptr = &post_args_ptr->phil->state}});
                }
                if (post_args_ptr->nick->has_state()) {
                    members[1]->post(
                        PostArgs{.extras = ui::StringPtrPostArgs{
                                     .ptr = &post_args_ptr->nick->state}});
                }
            }
        }
    }
};
} // namespace ui