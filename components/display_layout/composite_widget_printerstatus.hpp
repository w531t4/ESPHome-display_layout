// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "argsbag.hpp"
#include "base_widget_composite.hpp"
#include "base_widget_text_numeric.hpp"
#include "base_widget_text_string.hpp"
#include "ui_colors.hpp"

namespace ui {
class PrinterStatusWidget : public CompositeWidget<2> {
  private:
    static constexpr const char *TAG = "ui_widget_printerstatus";
    inline static const std::string LABEL = "PRINT";
    static constexpr int HIDDEN_VALUE = -1;
    static constexpr int LABEL_X_OFFSET = 8;
    static constexpr int VALUE_Y_OFFSET = 10;

  public:
    void initialize(const InitArgs &a) override {
        CompositeWidget<2>::initialize(a);
        if (!a.font.has_value()) {
            ESP_LOGE(
                TAG,
                "[widget=%s] initialize(): Required font parameter missing",
                this->get_name().c_str());
            return;
        }
        if (!*a.font) {
            ESP_LOGE(TAG,
                     "[widget=%s] initialize(): Required font must not "
                     "be nullptr",
                     this->get_name().c_str());
            return;
        }
        if (!a.font2.has_value()) {
            ESP_LOGE(
                TAG,
                "[widget=%s] initialize(): Required font2 parameter missing",
                this->get_name().c_str());
            return;
        }
        if (!*a.font2) {
            ESP_LOGE(TAG,
                     "[widget=%s] initialize(): Required font2 must not "
                     "be nullptr",
                     this->get_name().c_str());
            return;
        }

        members[0] = std::make_unique<StringWidget<6>>();
        members[0]->initialize(InitArgs{.it = a.it,
                                        .id = a.id + "[label]",
                                        .anchor =
                                            ui::Coord(a.anchor.x + LABEL_X_OFFSET,
                                                      a.anchor.y),
                                        .font = *a.font2,
                                        .font_color = PINK});

        members[1] = std::make_unique<NumericWidget<int, 3>>();
        members[1]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[value]",
                     .anchor = ui::Coord(a.anchor.x, a.anchor.y + VALUE_Y_OFFSET),
                     .font = a.font,
                     .font_color = PINK,
                     .fmt = std::string("%02d"),
                     .extras = ArgsBag::of(TextInitArgs<int>{
                         .trim_pixels_top = 9,
                         .trim_pixels_bottom = 9,
                         .hide_if_equal_val = HIDDEN_VALUE})});
        initialized = true;
    }

    void post(const PostArgs &args) override {
        if (!initialized)
            return;

        const NumericPostArgs<int> *post_args_ptr =
            std::any_cast<const NumericPostArgs<int>>(&args.extras);
        if (post_args_ptr == nullptr)
            return;

        members[0]->post(
            PostArgs{.extras = ui::StringPtrPostArgs{.ptr = &LABEL}});
        members[1]->post(
            PostArgs{.extras = ui::NumericPostArgs<int>{
                .value = post_args_ptr->value}});
    }

    void update() override {
        if (!initialized)
            return;

        const bool label_was_visible = members[0] && members[0]->is_visible();

        for (auto &ptr : members) {
            if (ptr && ptr->is_enabled())
                ptr->update();
        }

        const bool has_value = members[1] && members[1]->is_visible();
        if (!has_value && this->is_visible()) {
            if (members[0] && members[0]->is_visible()) {
                members[0]->blank();
                members[0]->set_visible(false);
            }
            this->set_visible(false);
        } else if (has_value && !(this->is_visible())) {
            if (members[0] && !(members[0]->is_visible())) {
                members[0]->set_visible(true);
            }
            this->set_visible(true);
        }
    }
};
} // namespace ui
