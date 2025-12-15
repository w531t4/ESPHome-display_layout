#pragma once
#include "ui_shared.hpp"
#include "ui_widget_dyntext_string.hpp"

namespace ui {
struct TwitchStringComponents {
    std::string user;
    std::string message;
};
struct MultiColorString {
    virtual void update_colors(esphome::Color &user,
                               esphome::Color &message) = 0;
};
TwitchStringComponents split_twitch_string(const std::string &input) {
    TwitchStringComponents result;
    std::size_t pos = input.find(':'); // Find the first ':' character
    if (pos != std::string::npos) {
        result.user = input.substr(0, pos); // Everything before ':' is the user
        std::size_t msg_start =
            pos + 1; // Everything after ':' (skipping a following
                     // space if present) is the message
        if (msg_start < input.size() && input[msg_start] == ' ')
            ++msg_start; // skip leading space after the colon
        result.message = input.substr(msg_start);
    } else {
        result.message =
            input; // No ':' found; treat the whole thing as the message
    }
    return result;
}

template <std::size_t BufSize>
class TwitchStringWidget : public DynStringWidget<BufSize>,
                           public MultiColorString {
  private:
    esphome::Color color_user;
    esphome::Color color_message;
    TwitchStringComponents abc;

  public:
    void prep(std::string value, const char *fmt) override {
        DynStringWidget<BufSize>::prep(value, fmt);
        this->abc = split_twitch_string(this->buf.data());
    }
    void update_colors(esphome::Color &user, esphome::Color &message) {
        this->color_user = user;
        this->color_message = message;
    }

    void write() override {
        if (this->abc.user == "" && this->abc.message == "")
            return;
        const int y = this->anchor.y - this->trim_pixels_top;
        if (this->use_max_width_as_width && this->right_align) {
            // printf will start drawing at the first pixel of a character,
            // ignoring leading whitespace in buffer.
            const int curr_buf_width = this->bounds(this->buf.data()).w;
            const int x_draw =
                this->anchor.x + (this->max_width - curr_buf_width);
            ui::printf_dual(this->it, this->font, x_draw, y,
                            (this->abc.user + ": ").c_str(), WHITE,
                            this->abc.message.c_str(), YELLOW, this->prev_box);
        } else {
            ui::printf_dual(this->it, this->font, this->anchor.x, y,
                            (this->abc.user + ": ").c_str(), WHITE,
                            this->abc.message.c_str(), YELLOW, this->prev_box);
        }
    }
};
} // namespace ui
