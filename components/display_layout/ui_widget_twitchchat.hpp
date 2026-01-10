// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "ui_colors.hpp"
#include "ui_shared.hpp"
#include "ui_widget_twitchstring.hpp"
#include "ui_widgetcomposite.hpp"

namespace ui {
struct TwitchChatPostArgs {
    std::string row1;
    std::string row2;
    std::string row3;
};
struct TwitchChatInitArgs {
    std::optional<int> pixels_per_character;
};

template <std::size_t BufSize>
class TwitchChatWidget : public CompositeWidget<3> {
  private:
    int pixels_per_character = 6;
    int pixel_capacity = -1;

  public:
    void initialize(const InitArgs &a) override {
        CompositeWidget<3>::initialize(a);
        if (auto *t = a.extras.get<TwitchChatInitArgs>()) {
            this->pixels_per_character = t->pixels_per_character.value_or(4);
            if (this->pixels_per_character <= 0) {
                this->pixels_per_character = 1;
            }
        }
        const esphome::Color font_color = YELLOW;

        members[0] = std::make_unique<TwitchStringWidget<BufSize>>();
        members[0]->initialize(InitArgs{.it = a.it,
                                        .id = a.id + "[line1]",
                                        .anchor = ui::Coord(anchor.x, anchor.y),
                                        .font = *a.font,
                                        .font_color = font_color});
        members[1] = std::make_unique<TwitchStringWidget<BufSize>>();
        members[1]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[line2]",
                     .anchor = ui::Coord(anchor.x, anchor.y + 11),
                     .font = *a.font,
                     .font_color = font_color});
        members[2] = std::make_unique<TwitchStringWidget<BufSize>>();
        members[2]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[line3]",
                     .anchor = ui::Coord(anchor.x, anchor.y + 21),
                     .font = *a.font,
                     .font_color = font_color});
        this->set_capacity(100, true);
        initialized = true;
    }
    void set_capacity(const std::size_t cap, const bool preserve = true) {
        if (!initialized)
            return;
        if (cap < 1)
            return;
        int num_chars = static_cast<int>(cap / this->pixels_per_character);
        for (auto &p : members) {
            if (!p)
                continue;
            // All three members are created as DynStringWidget<BufSize> in
            // initialize()
            auto *row = static_cast<TwitchStringWidget<BufSize> *>(p.get());
            row->set_capacity(num_chars, preserve);
        }
        this->blank();
        this->write();
        this->pixel_capacity = cap;
    }

    const size_t get_capacity() const { return this->pixel_capacity; }

    void post(const PostArgs &args) override {
        if (args.extras.has_value()) {
            const TwitchChatPostArgs *post_args_ptr =
                std::any_cast<const TwitchChatPostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                members[0]->post(PostArgs{.extras = ui::StringPtrPostArgs{
                                              .ptr = &post_args_ptr->row1}});
                members[1]->post(PostArgs{.extras = ui::StringPtrPostArgs{
                                              .ptr = &post_args_ptr->row2}});
                members[2]->post(PostArgs{.extras = ui::StringPtrPostArgs{
                                              .ptr = &post_args_ptr->row3}});
            }
        }
    }
};
} // namespace ui
