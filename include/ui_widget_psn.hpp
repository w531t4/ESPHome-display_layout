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
        members[0]->initialize(InitArgs{.it = a.it,
                                        .id = a.id + "[phil]",
                                        .anchor = ui::Coord(anchor.x, anchor.y),
                                        .font = *a.font,
                                        .font_color = GREEN});
        members[1] = std::make_unique<StringWidget<2>>(); // Nick
        members[1]->initialize(
            InitArgs{.it = a.it,
                     .id = a.id + "[nick]",
                     .anchor = ui::Coord(anchor.x, anchor.y + 20),
                     .font = *a.font,
                     .font_color = PINK});
        initialized = true;
    }
    void post(const PostArgs &args) {
        if (args.extras.has_value()) {
            const PSNPostArgs *post_args_ptr =
                std::any_cast<const PSNPostArgs>(&args.extras);
            if (post_args_ptr != nullptr) {
                bool should_psn_widget_show = false;
                if (ui::txt_sensor_has_healthy_state(post_args_ptr->phil)) {
                    if (!members[0]->is_enabled())
                        members[0]->set_enabled(true);
                    // members[0]->post(PostArgs{.extras =
                    // ui::StringPostArgs{.value =
                    // post_args_ptr->phil->state.substr(0, 1)}});
                    members[0]->post(PostArgs{.extras = ui::StringPostArgs{
                                                  .value = std::string("P")}});
                    should_psn_widget_show = true;
                } else {
                    if (members[0]->is_enabled()) {
                        members[0]->blank();
                        members[0]->set_enabled(false);
                    }
                }
                if (ui::txt_sensor_has_healthy_state(post_args_ptr->nick)) {
                    if (!members[1]->is_enabled())
                        members[1]->set_enabled(true);
                    // members[1]->post(PostArgs{.extras =
                    // ui::StringPostArgs{.value =
                    // post_args_ptr->nick->state.substr(0, 1)}});
                    members[1]->post(PostArgs{.extras = ui::StringPostArgs{
                                                  .value = std::string("N")}});
                    should_psn_widget_show = true;
                } else {
                    if (members[1]->is_enabled()) {
                        members[1]->blank();
                        members[1]->set_enabled(false);
                    }
                }
                const bool current_state = this->is_enabled();
                if (should_psn_widget_show && !current_state) {
                    this->blank();
                    this->set_enabled(true);
                } else if (!should_psn_widget_show && current_state) {
                    this->blank();
                    this->set_enabled(false);
                }
            }
        }
    }
};
} // namespace ui